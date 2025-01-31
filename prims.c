// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <string.h>
#include "prims.h"

#define D_SAVE() Cell saved_d = dataPop(c, m)
#define D_RESTORE() dataPush(c, m, saved_d)

#define R_SAVE() Cell saved_r = funcPop(c, m)
#define R_RESTORE() funcPush(c, m, saved_r)

PrimitiveData primTable[PRIM_NUM] = PRIM_TABLE_DEFAULT;

MAKEPRIM(colon) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: word created with no name\n"); }
    char *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, NULL, 0);

    m[c->compile_state_ptr] = 1;
}
MAKEPRIM(colonAnonymous) {
    makeWord(c, m, "\0", 0, 0, NULL, 0);
    m[c->compile_state_ptr] = 1;
}
MAKEPRIM(semicolon){
    appendWord(c, m, CA(t_end), 1);
    m[c->compile_state_ptr] = 0;
    if (m[m[c->dict_pos_ptr]+1] == 0)
        dataPush(c, m, m[c->dict_pos_ptr]);
}
MAKEPRIM(immediate) {
    m[m[c->dict_pos_ptr]+2] |= 1 << 24;
}
MAKEPRIM(leftbracket) {
    m[c->compile_state_ptr] = 0;
}
MAKEPRIM(rightbracket) {
    m[c->compile_state_ptr] = 1;
}
MAKEPRIM(recurse) {
    dataPush(c, m, m[c->dict_pos_ptr]);
    PRIM(comma)(c, m);
}
MAKEPRIM(leftparen) {
    int w_size = advanceTo(&c->inter_str, ')', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { }
    else { c->inter_str++; }
}
MAKEPRIM(backslash) {
    int w_size = advanceTo(&c->inter_str, '\n', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { }
    else { c->inter_str++; }
}
/*---------------------------------------------*/
MAKEPRIM(exit) {
    appendWord(c, m, CA(t_end), 1);
}
MAKEPRIM(if) {
    Cell forward_ref = appendWord(c, m, CA(t_condreljump, 0), 2);
    dataPush(c, m, forward_ref);
}
MAKEPRIM(ahead) {
    Cell forward_ref = appendWord(c, m, CA(t_reljump, 0), 2);
    dataPush(c, m, forward_ref);
}
MAKEPRIM(then){
    Cell current_word = m[c->dict_pos_ptr];
    Cell word_size = 3;
    Cell data_size = m[m[c->dict_pos_ptr]+2] & 0x0000FFFF;
    Cell back_ref = dataPop(c, m);

    // What we're calculating is the position of the next word.
    // This works because, at minimum, we will have the end-of-word tag
    // so we know we don't jump outside the word.
    // We add 1 at the end because we're interested in the second field,
    // as back_ref is calculated based on second field as well,
    // so if we add one we can just substract them to get
    // the relative jump size.
    Cell future_loc = current_word+word_size+data_size;
    m[back_ref] = future_loc-(back_ref-1);
}
MAKEPRIM(begin) {
    Cell current_word = m[c->dict_pos_ptr];
    Cell word_size = 3;
    Cell data_size = m[m[c->dict_pos_ptr]+2] & 0x0000FFFF;
    Cell future_loc = current_word+word_size+data_size;
    dataPush(c, m, future_loc);
}
MAKEPRIM(again){
    Cell back_ref = dataPop(c, m);
    Cell last_cell = appendWord(c, m, CA(t_reljumpback, 0), 2);
    last_cell -= 1;
 m[last_cell+1] = last_cell-back_ref;
}
MAKEPRIM(until){
    Cell back_ref = dataPop(c, m);
    Cell last_cell = appendWord(c, m, CA(t_condreljumpback, 0), 2);
    last_cell -= 1;
 m[last_cell+1] = last_cell-back_ref;
}
/*---------------------------------------------*/
MAKEPRIM(variablerawsize) {
    Cell w_cell = dataPop(c, m);
    dataPush(c, m, m[w_cell+2] & 0xFFFF);
}
MAKEPRIM(emptyvariable) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'VARIABLE' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, NULL, 0);
}
MAKEPRIM(variable) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'VARIABLE' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    unsigned size = dataPop(c, m);
    makeWord(c, m, lorig, w_size, 0, NULL, 0);
    appendWord(c, m, CA(t_num, m[c->dict_pos_ptr]+7), 2); // +3, pointer to first empty cell
    appendWord(c, m, CA(t_end, t_end), 2); // +5, Two ends to be replaced by an absolute jump
    for (unsigned i = 0; i < size; i++) {
        appendWord(c, m, CA(0), 1);
    }
}
MAKEPRIM(literal) {
    Cell popped_num = dataPop(c, m);
    appendWord(c, m, CA(t_num, popped_num), 2);
}
MAKEPRIM(comma) {
    Cell val = dataPop(c, m);
    appendWord(c, m, CA(val), 1);
}
MAKEPRIM(allot) {
    int size = dataPop(c, m);
    for (int i = 0; i < size; i++) {
        appendWord(c, m, CA(0), 1);
    }
}
MAKEPRIM(here) {
 dataPush(c, m, m[c->dict_pos_ptr]+3+(m[m[c->dict_pos_ptr]+2] & 0xFFFF));
}
MAKEPRIM(getwordbodysize) {
    Cell xt = dataPop(c, m);
    dataPush(c, m, (m[xt+2] & 0xFFFF)-4);
}
MAKEPRIM(getwordbody) {
    Cell xt = dataPop(c, m);
    //Must have standard structure created by VARIABLE
    if ((m[xt+2] & 0xFFFF) > 0) dataPush(c, m, xt+7);
    else dataPush(c, m, 0);
}
MAKEPRIM(worddoesprim) {
    Cell current_word = m[c->dict_pos_ptr];
    Cell pc = funcPeek(c, m); // DO NOT CALL WORDDOESPRIM EXCEPT THROUGH THE DICTIONARY INTERFACE! IT DOES CALLSTACK MANIPULATION!
    m[current_word+5] = t_absjump; // replace first dummy END created by VAR with a jump
    m[current_word+6] = pc+2; //replace other dummy END with jump value
}
MAKEPRIM(worddoes) {
    Cell appended_w = findWord(c, m, 'c', "DOES>PRIM", strlen("DOES>PRIM"));
    appendWord(c, m, CA(appended_w), 1);
    appendWord(c, m, CA(t_end_notailcall), 1); // Tail calls break the kind of callstack manipulation we do with worddoesprim
}
MAKEPRIM(move) {
    Cell n = dataPop(c, m);
    Cell adr2 = dataPop(c, m);
    Cell adr1 = dataPop(c, m);
    for (unsigned i = 0; i < n; i++) {
        m[adr2+i] = m[adr1+i];
    }
}
/*---------------------------------------------*/
MAKEPRIM(bracket_char_bracket) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: '[CHAR]' called with no name\n"); }
    char *lorig = c->inter_str-w_size;
    appendWord(c, m, CA(t_num, lorig[0]), 2);
}
MAKEPRIM(char) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'CHAR' called with no name\n"); }
    char *lorig = c->inter_str-w_size;
    dataPush(c, m, lorig[0]);
}
MAKEPRIM(word) {
    int w_size = advanceTo(&c->inter_str, (char)dataPop(c, m), 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'WORD' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string);
}
MAKEPRIM(parse) {
    int w_size = advanceTo(&c->inter_str, (char)dataPop(c, m), 0);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'PARSER' called with no name\n"); }
    char *lorig = c->inter_str-w_size;
    c->inter_str += 1;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string+1);
    dataPush(c, m, m[created_string]-1);
}
MAKEPRIM(parse_name) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'PASER-NAME' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string+1);
    dataPush(c, m, m[created_string]);
}
MAKEPRIM(find) {
    Cell obtained_str = dataPop(c, m);
    Cell found_word = findWord(c, m, 'n', &m[obtained_str+1], m[obtained_str]-1);
    if (found_word == 0) { dataPush(c, m, obtained_str); dataPush(c, m, 0); }
    else { dataPush(c, m, found_word); dataPush(c, m, ((m[found_word+2] >> 24) != 0) ? 1 : -1); }
}
MAKEPRIM(who) {
    Cell obtained_xt = dataPop(c, m);
    dataPush(c, m, m[obtained_xt+1]);
}
MAKEPRIM(is) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'IS' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    Cell assigned_word = findWord(c, m, 'c', lorig, w_size);

    Cell popped_xt = dataPop(c, m);
    if ((m[assigned_word+2] & 0xFFFF) >= 1)
        m[assigned_word+3] = popped_xt;
    else
        printf("Error: Can't assign to non-deferred word with inappropiate size %u\n", m[assigned_word]);
}
MAKEPRIM(defer) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'DEFER' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, CA(t_end, t_end), 2);
}
MAKEPRIM(postpone) {
    int w_size = advanceTo(&c->inter_str, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("Warning: 'POSTPONE' called with no name\n"); }
    char *lorig = c->inter_str-w_size;

    Cell found_word = findWord(c, m, 'c', lorig, w_size);

    appendWord(c, m, CA(found_word), 1);
}
MAKEPRIM(execute) {
    Cell popped_xt = dataPop(c, m);
    executeWord(c, m, popped_xt);
}
/*---------------------------------------------*/
MAKEPRIM(add){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2+w1);
}
MAKEPRIM(minus){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2-w1);
}
MAKEPRIM(mult){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2*w1);
}
MAKEPRIM(div){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2/w1);
}
MAKEPRIM(mod){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2%w1);
}
MAKEPRIM(invert){
 Cell w1 = dataPop(c, m);
 dataPush(c, m, -w1);
}
MAKEPRIM(rshift){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2>>w1);
}
MAKEPRIM(lshift){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2<<w1);
}
MAKEPRIM(max){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, (w1>w2) ? w1 : w2);
}
MAKEPRIM(min){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, (w1<w2) ? w1 : w2);
}
MAKEPRIM(negate){
 Cell w1 = dataPop(c, m);
 dataPush(c, m, w1 ? BOOL(0) : BOOL(1));
}
MAKEPRIM(and){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2&w1);
}
MAKEPRIM(or){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2|w1);
}
MAKEPRIM(xor){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, w2^w1);
}
MAKEPRIM(abs){
 Cell w1 = dataPop(c, m);
 dataPush(c, m, ((int)w1 < 0) ? -w1 : w1);
}
MAKEPRIM(eq){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2==w1));
}
MAKEPRIM(neq){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2!=w1));
}
MAKEPRIM(le){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2<w1));
}
MAKEPRIM(leq){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2<=w1));
}
MAKEPRIM(gr){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2>w1));
}
MAKEPRIM(geq){
 Cell w1 = dataPop(c, m);
 Cell w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2>=w1));
}
/*---------------------------------------------*/
MAKEPRIM(fetch) {
    dataPush(c, m, m[dataPop(c, m)]);
}
MAKEPRIM(store) {
    Cell adr = dataPop(c, m);
    Cell val = dataPop(c, m);
    m[adr] = val;
}
MAKEPRIM(rget) {
	R_SAVE();
    funcPush(c, m, dataPop(c, m));
	R_RESTORE();
}
MAKEPRIM(rsend) {
	R_SAVE();
    dataPush(c, m, funcPop(c, m));
	R_RESTORE();
}
/*---------------------------------------------*/
MAKEPRIM(2fetch) {
    Cell adr = dataPop(c, m);
    dataPush(c, m, m[adr]);
    dataPush(c, m, m[adr+1]);
}
MAKEPRIM(2store) {
    Cell adr = dataPop(c, m);
    Cell val2 = dataPop(c, m);
    Cell val1 = dataPop(c, m);
    // Yes, they are stored in reverse according to the standard
    m[adr] = val2;
    m[adr+1] = val1;
}
MAKEPRIM(2rget) {
	R_SAVE();
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    funcPush(c, m, w1);
    funcPush(c, m, w2);
	R_RESTORE();
}
MAKEPRIM(2rsend) {
	R_SAVE();
    Cell w2 = funcPop(c, m);
    Cell w1 = funcPop(c, m);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
	R_RESTORE();
}
/*---------------------------------------------*/
MAKEPRIM(count) {
    Cell obtained_str = dataPop(c, m);
    dataPush(c, m, obtained_str+1);
    dataPush(c, m, m[obtained_str]-1);
}
MAKEPRIM(bl) {
    dataPush(c, m, 0x20);
}
MAKEPRIM(emit){
    unsigned char ch = dataPop(c, m);
    if(ch < 0x21 || ch > 0x7E) {
        printf("[0x%X]", ch);
    } else {
        printf("%c", ch);
    }
}
MAKEPRIM(type){
    Cell size = dataPop(c, m);
    Cell str = dataPop(c, m);
    if (size == 0) {
        printf("[EMPTY_STR]");
    }
    for (unsigned i = 0; i < size; i++) {
        printf("%c", (unsigned char)m[str+i]);
    }
}
MAKEPRIM(cr) {
    printf("\n");
}
MAKEPRIM(spaces) {
    for (unsigned i = 0; i < dataPop(c, m); i++) {
        printf(" ");
    }
}
MAKEPRIM(dot) {
    printf(" %d", dataPop(c, m));
}
MAKEPRIM(udot) {
    printf(" %u", dataPop(c, m));
}
MAKEPRIM(dotmem) {
    printMemory(m, 0, 0x1000, 0x10);
}
MAKEPRIM(dotstack) {
    unsigned stacksize = m[c->dstack_ptr]-(c->dstack_start);
    printf(" | s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %d", m[c->dstack_start+i]);
    }
    printf(" |");
}
MAKEPRIM(udotstack) {
    unsigned stacksize = m[c->dstack_ptr]-(c->dstack_start);
    printf(" | s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %u", m[c->dstack_start+i]);
    }
    printf(" |");
}
MAKEPRIM(dotstackreturn) {
    unsigned stacksize = m[c->fstack_ptr]-(c->fstack_start);
    printf(" R| s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %d", m[c->fstack_start+i]);
    }
    printf(" |");
}
MAKEPRIM(udotstackreturn) {
    unsigned stacksize = m[c->fstack_ptr]-(c->fstack_start);
    printf(" R| s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %u", m[c->fstack_start+i]);
    }
    printf(" |");
}
/*---------------------------------------------*/
// ( w -- )
MAKEPRIM(drop) {
    dataPop(c, m);
}
// ( w1 w2 -- w2)
MAKEPRIM(nip) {
    Cell w2 = dataPop(c, m);
    dataPop(c, m);
    dataPush(c, m, w2);
}
// ( w -- w w)
MAKEPRIM(dup) {
    Cell w1 = dataPeek(c, m);
    dataPush(c, m, w1);
}
// ( w1 w2 -- w1 w2 w1)
MAKEPRIM(over) {
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPeek(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w1);
}
// ( w1 w2 -- w2 w1)
MAKEPRIM(swap) {
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w1);
}
// ( w1 w2 w3 -- w2 w3 w1)
MAKEPRIM(rot) {
    Cell w3 = dataPop(c, m);
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w3);
    dataPush(c, m, w1);
}
// ( w1 w2 -- w2 w1 w2)
MAKEPRIM(tuck) {
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
}
/*---------------------------------------------*/
// ( w1 w2 -- )
MAKEPRIM(2drop) {
    dataPop(c, m);
    dataPop(c, m);
}
// ( w1 w2 w3 w4 -- w3 w4)
MAKEPRIM(2nip) {
    Cell w4 = dataPop(c, m);
    Cell w3 = dataPop(c, m);
    dataPop(c, m);
    dataPop(c, m);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
}
// ( w1 w2 -- w1 w2 w1 w2)
MAKEPRIM(2dup) {
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPeek(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
}
// ( w1 w2 w3 w4 -- w1 w2 w3 w4 w1 w2)
MAKEPRIM(2over) {
    Cell w4 = dataPop(c, m);
    Cell w3 = dataPop(c, m);
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPeek(c, m);
    dataPush(c, m, w2);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
}
// ( w1 w2 w3 w4 -- w3 w4 w1 w2)
MAKEPRIM(2swap) {
    Cell w4 = dataPop(c, m);
    Cell w3 = dataPop(c, m);
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
}
// ( w1 w2 w3 w4 w5 w6 -- w3 w4 w5 w6 w1 w2)
MAKEPRIM(2rot) {
    Cell w6 = dataPop(c, m);
    Cell w5 = dataPop(c, m);
    Cell w4 = dataPop(c, m);
    Cell w3 = dataPop(c, m);
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
    dataPush(c, m, w5);
    dataPush(c, m, w6);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
}
// ( w1 w2 w3 w4 -- w3 w4 w1 w2 w3 w4)
MAKEPRIM(2tuck) {
    Cell w4 = dataPop(c, m);
    Cell w3 = dataPop(c, m);
    Cell w2 = dataPop(c, m);
    Cell w1 = dataPop(c, m);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
    dataPush(c, m, w1);
    dataPush(c, m, w2);
    dataPush(c, m, w3);
    dataPush(c, m, w4);
}
/*---------------------------------------------*/
// R( w -- )
MAKEPRIM(rdrop) {
	R_SAVE();
    funcPop(c, m);
	R_RESTORE();
}
// R( w1 w2 -- w2)
MAKEPRIM(rnip) {
	R_SAVE();
    Cell w2 = funcPop(c, m);
    funcPop(c, m);
    funcPush(c, m, w2);
	R_RESTORE();
}
// R( w -- w w)
MAKEPRIM(rdup) {
	R_SAVE();
    Cell w1 = funcPeek(c, m);
    funcPush(c, m, w1);
	R_RESTORE();
}
// R( w1 w2 -- w1 w2 w1)
MAKEPRIM(rover) {
	R_SAVE();
    Cell w2 = funcPop(c, m);
    Cell w1 = funcPeek(c, m);
    funcPush(c, m, w2);
    funcPush(c, m, w1);
	R_RESTORE();
}
// R( w1 w2 -- w2 w1)
MAKEPRIM(rswap) {
	R_SAVE();
    Cell w2 = funcPop(c, m);
    Cell w1 = funcPop(c, m);
    funcPush(c, m, w2);
    funcPush(c, m, w1);
	R_RESTORE();
}
// R( w1 w2 w3 -- w2 w3 w1)
MAKEPRIM(rrot) {
	R_SAVE();
    Cell w3 = funcPop(c, m);
    Cell w2 = funcPop(c, m);
    Cell w1 = funcPop(c, m);
    funcPush(c, m, w2);
    funcPush(c, m, w3);
    funcPush(c, m, w1);
	R_RESTORE();
}
// R( w1 w2 -- w2 w1 w2)
MAKEPRIM(rtuck) {
	R_SAVE();
    Cell w2 = funcPop(c, m);
    Cell w1 = funcPop(c, m);
    funcPush(c, m, w2);
    funcPush(c, m, w1);
    funcPush(c, m, w2);
	R_RESTORE();
}

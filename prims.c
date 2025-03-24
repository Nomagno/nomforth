// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prims.h"

#define D_SAVE() Cell saved_d = dataPop(c, m)
#define D_RESTORE() dataPush(c, m, saved_d)

#define R_SAVE() Cell saved_r = funcPop(c, m)
#define R_RESTORE() funcPush(c, m, saved_r)

PrimitiveData primTable[PRIM_NUM] = PRIM_TABLE_DEFAULT;

/* The forth core words: declare a new word, and end it */
MAKEPRIM(colon) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: word created with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);

    m[c->compile_state_ptr] = 1;
}
MAKEPRIM(colonAnon) {
    makeWord(c, m, (Cell[]){0}, 0, 0, 0, 0, NULL, 0);
    m[c->compile_state_ptr] = 1;
    dataPush(c, m, m[c->dict_pos_ptr]);
}
MAKEPRIM(semicolon){
    appendWord(c, m, CA(t_end), 1);
    m[c->compile_state_ptr] = 0;
}
/* Comments*/
/*---------------------------------------------*/
MAKEPRIM(leftparen) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ')', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { }
    else { c->inter_str++; }
}
MAKEPRIM(backslash) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, '\n', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { }
    else { c->inter_str++; }
}
/* Quit the interpreter immediatel*/
/*---------------------------------------------*/
MAKEPRIM(bye) {
    printf("\nThanks for using nomForth.\n");
    exit(0);
}

/* Manipulate words*/
/*---------------------------------------------*/
MAKEPRIM(emptyword) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'EMPTY_WORD' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);
}
MAKEPRIM(create) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'CREATE' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);
    m[m[c->dict_pos_ptr]+2] |= 0x80000000; /*Mark as variable by setting highest bit*/
    appendWord(c, m, CA(t_num, m[c->dict_pos_ptr]+7), 2); // +3, pointer to first empty cell
    appendWord(c, m, CA(t_end, t_end), 2); // +5, Two ends to be replaced by an absolute jump
}
MAKEPRIM(comma) {
    Cell val = dataPop(c, m);
    appendWord(c, m, CA(val), 1);
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
/* Memory manipulation, returnstack-datastack interaction */
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
/* Reading data from the input stream*/
/*---------------------------------------------*/
MAKEPRIM(b_char_b) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: '[CHAR]' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;
    appendWord(c, m, CA(t_num, lorig[0]), 2);
}
MAKEPRIM(char) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'CHAR' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;
    dataPush(c, m, lorig[0]);
}
MAKEPRIM(word) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, (char)dataPop(c, m), 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'WORD' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string);
}
MAKEPRIM(parse) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, (char)dataPop(c, m), 0);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'PARSER' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;
    c->inter_str += 1;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string);
}
MAKEPRIM(parse_name) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'PASER-NAME' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    Cell created_string = addToPad(c, m, lorig, w_size);
    dataPush(c, m, created_string);
}
MAKEPRIM(find) {
    Cell obtained_str = dataPop(c, m);
    Cell found_word = findWord(c, m, 'n', &m[obtained_str+1], m[obtained_str]-1);
    if (found_word == 0) { dataPush(c, m, obtained_str); dataPush(c, m, 0); }
    else { dataPush(c, m, found_word); dataPush(c, m, ((m[found_word+2] >> 24) != 0) ? 1 : -1); }
}
MAKEPRIM(is) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'IS' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    Cell assigned_word = findWord(c, m, 'n', lorig, w_size);

    Cell popped_xt = dataPop(c, m);
    if ((m[assigned_word+2] & 0x0000FFFF) >= 1)
        m[assigned_word+3] = popped_xt;
    else
        printf("{ERROR: Can't assign to non-deferred word with inappropiate size %u}\n", m[assigned_word]);
}
MAKEPRIM(defer) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'DEFER' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    makeWord(c, m, lorig, w_size, 0, 0, 0, CA(t_end, t_end), 2);
}
MAKEPRIM(postpone) {
    int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
    if (w_size < 0) { w_size = -w_size; }
    if (w_size == 0) { printf("WARNING: 'POSTPONE' called with no name\n"); }
    Cell *lorig = c->inter_str-w_size;

    Cell found_word = findWord(c, m, 'n', lorig, w_size);

    appendWord(c, m, CA(found_word), 1);
}

MAKEPRIM(getchar){
    uint8_t w1 = getchar();
    if (w1 == '\n') w1 = 0;
    dataPush(c, m, w1);
    if (w1 == '\n') return;
    else while (getchar() != '\n');
}

MAKEPRIM(getnum){
    Cell w1;
    _Bool found = scanf("%d", &w1);
    while (getchar() != '\n');

    if (found) dataPush(c, m, w1);
    else       dataPush(c, m, -1);
}

MAKEPRIM(evaluate){
    Cell s = dataPop(c, m);
    Cell adr = dataPop(c, m);
    // Set up our own stack frame on the native stack
    // Evaluate is the only word that can
    //  result in a native stack overflow,
    //  so please don't somehow use it
    //  recursively.
    Cell *inter_max = c->inter_max;
    Cell *inter_min = c->inter_min;
    Cell *inter_str = c->inter_str;

    Cell pc = m[c->program_counter_ptr];
    interpret(c, m, &m[adr], s, 0);
    m[c->program_counter_ptr] = pc;

    c->inter_max = inter_max;
    c->inter_min = inter_min;
    c->inter_str = inter_str;
}

/*Interface to Offset-based allocator in oa.h*/
/*---------------------------------------------*/
MAKEPRIM(heap_init){
    OA_init(m+c->heap_start, 0x2000);
}
MAKEPRIM(defrag){
    OA_defrag();
}

MAKEPRIM(allocate){
    Cell s = dataPop(c, m);
    if (s == 0) return;
    Cell mem = OA_malloc(s);
    mem = (mem == NIL) ? 0 : (mem + c->heap_start);
    dataPush(c, m, mem);
}

MAKEPRIM(free){
    Cell mem = dataPop(c, m);
    mem = (mem == 0) ? NIL : (mem - c->heap_start);
    OA_free(mem);
}


/* From here on it's all trivial boilerplate for C arithmetic operations and I/O*/
/*---------------------------------------------*/
MAKEPRIM(add){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, w2+w1);
}
MAKEPRIM(minus){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, w2-w1);
}
MAKEPRIM(mult){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, w2*w1);
}
MAKEPRIM(div){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, w2/w1);
}
MAKEPRIM(mod){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, w2%w1);
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
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, (w1>w2) ? w1 : w2);
}
MAKEPRIM(min){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, (w1<w2) ? w1 : w2);
}
MAKEPRIM(arith_not){
 Cell w1 = dataPop(c, m);
 dataPush(c, m, -w1);
}
MAKEPRIM(bitwise_not){
 Cell w1 = dataPop(c, m);
 dataPush(c, m, ~w1);
}
MAKEPRIM(logical_not){
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
 int32_t w1 = dataPop(c, m);
 dataPush(c, m, (w1 < 0) ? -w1 : w1);
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
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2<w1));
}
MAKEPRIM(leq){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2<=w1));
}
MAKEPRIM(gr){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2>w1));
}
MAKEPRIM(geq){
 int32_t w1 = dataPop(c, m);
 int32_t w2 = dataPop(c, m);
 dataPush(c, m, BOOL(w2>=w1));
}
/*Printing strings, numbers, memory, etc.*/
/*---------------------------------------------*/
MAKEPRIM(emit){
    unsigned char ch = dataPop(c, m);
    if (ch >= 0x20 && ch <= 0x7E) {
        printf("%c", ch);
    } else if (ch == 0x0A) {
        printf("%c", ch);
    } else {
        printf("[0x%X]", ch);
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
    Cell x = dataPop(c, m);
    for (unsigned i = 0; i < x; i++) {
        printf(" ");
    }
}
MAKEPRIM(dot) {
    printf(" %d", dataPop(c, m));
}
MAKEPRIM(udot) {
    printf(" %u", dataPop(c, m));
}
MAKEPRIM(xdot) {
    printf(" %08X", dataPop(c, m));
}
MAKEPRIM(dotmem) {
    Cell start = dataPop(c, m);
    Cell end = dataPop(c, m);
    Cell step = dataPop(c, m);
    printMemory(m, start, end, step);
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
MAKEPRIM(dotstackret) {
    R_SAVE();
    unsigned stacksize = m[c->fstack_ptr]-(c->fstack_start);
    printf(" R| s <%d>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %d", m[c->fstack_start+i]);
    }
    printf(" |");
    R_RESTORE();
}
MAKEPRIM(udotstackret) {
    unsigned stacksize = m[c->fstack_ptr]-(c->fstack_start);
    printf(" R| s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %u", m[c->fstack_start+i]);
    }
    printf(" |");
}
/*Simple data stack manipulation*/
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
/* Pair-wise data stack manipulation*/
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
/* Return stack manipulation */
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

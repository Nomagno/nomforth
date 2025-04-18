// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include "prims.h"

#define D_SAVE() Cell saved_d = dataPop(c, m)
#define D_RESTORE() dataPush(c, m, saved_d)

#define R_SAVE() Cell saved_r = funcPop(c, m)
#define R_RESTORE() funcPush(c, m, saved_r)

PrimitiveData primTable[PRIM_NUM] = PRIM_TABLE_DEFAULT;

#define WARNING(name) printf("WARNING: '%s' called with no name\n", #name)
#define C_LOR() Cell *lorig = c->inter_str-w_size;
/* CONSUMER is a macro to reflect a very common forth pattern:
 * A word that hijacks the input stream, advances it until the next character
 * of a certain type is found (skipping all initial consecutive occurences of the
 * character), and depending on if 0 or non-0 significant characters were advanced,
 * performs different actions. Then, performs an action unconditionally. */
#define CONSUMER(ch, post_action, nonzero_size, ...) \
    int w_size = advanceTo(&c->inter_str, c->inter_max, ch, 1); \
    if (w_size < 0) { w_size = -w_size; } \
    if (w_size == 0) { __VA_ARGS__; } \
    else { nonzero_size; } \
    post_action

static unsigned cell_strlen(Cell *i) { Cell *s; for (s = i; *s; ++s){}; return (s - i); }
static unsigned char_strlen(char *i) { char *s; for (s = i; *s; ++s){}; return (s - i); }

/* The forth core words: declare a new word, and end it */
MAKEPRIM(colon) {
    CONSUMER(' ', C_LOR(), , WARNING(colon));
    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);
    m[c->compile_state_ptr] = 1;
}
MAKEPRIM(colonAnon) {
    makeWord(c, m, (Cell[]){0}, 0, 0, 0, 0, NULL, 0);
    m[c->compile_state_ptr] = 1;
    dataPush(c, m, m[c->dict_pos_ptr]);
}
MAKEPRIM(semicolon){ appendWord(c, m, CA(t_end), 1); m[c->compile_state_ptr] = 0; }
/* Comments*/
/*---------------------------------------------*/
MAKEPRIM(leftparen) { CONSUMER(')', , c->inter_str++, ); }
MAKEPRIM(backslash) { CONSUMER('\n', , c->inter_str++, ); }
/* Quit the interpreter immediately*/
/*---------------------------------------------*/
MAKEPRIM(bye) { printf("\nThanks for using nomForth.\n"); exit(0); }

/* Manipulate words*/
/*---------------------------------------------*/
MAKEPRIM(emptyword) {
    CONSUMER(' ', C_LOR(), , WARNING(empty_word));
    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);
}
MAKEPRIM(create) {
    CONSUMER(' ', C_LOR(), , WARNING(create));
    makeWord(c, m, lorig, w_size, 0, 0, 0, NULL, 0);
    m[m[c->dict_pos_ptr]+2] |= 0x80000000; /*Mark as variable by setting highest bit*/
    appendWord(c, m, CA(t_num, m[c->dict_pos_ptr]+7), 2); // +3, pointer to first empty cell
    appendWord(c, m, CA(t_end, t_end), 2); // +5, Two ends to be replaced by an absolute jump
}
MAKEPRIM(comma) { Cell val = dataPop(c, m); appendWord(c, m, CA(val), 1); }
MAKEPRIM(worddoesprim) { // DO NOT CALL THE WORDDOESPRIM C PRIMITIVE DIRECTLY!
    Cell current_word = m[c->dict_pos_ptr];
    Cell pc = funcPeek(c, m); 
    m[current_word+5] = t_absjump; // replace first dummy END created by VAR with a jump
    m[current_word+6] = pc+2; //replace other dummy END with jump value
}
MAKEPRIM(worddoes) {
    Cell appended_w = findWord(c, m, 'c', "DOES>PRIM", char_strlen("DOES>PRIM"));
    appendWord(c, m, CA(appended_w), 1);
    appendWord(c, m, CA(t_end_notailcall), 1); // Tail calls break the kind of callstack manipulation we do with worddoesprim
}
/* Reading data from the input stream*/
/*---------------------------------------------*/
MAKEPRIM(b_char_b) {
    CONSUMER(' ', C_LOR(), , WARNING([char]));
    appendWord(c, m, CA(t_num, lorig[0]), 2);
}
MAKEPRIM(char) {
    CONSUMER(' ', C_LOR(), , WARNING(char));
    dataPush(c, m, lorig[0]);
}
MAKEPRIM(word) {
    CONSUMER((char)dataPop(c, m), C_LOR(), , WARNING(word))
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
    CONSUMER(' ', C_LOR(), , WARNING(parse_name));
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
    CONSUMER(' ', C_LOR(), , WARNING(is));
    Cell assigned_word = findWord(c, m, 'n', lorig, w_size);
    Cell popped_xt = dataPop(c, m);
    if ((m[assigned_word+2] & 0x0000FFFF) >= 1)
        m[assigned_word+3] = popped_xt;
    else
        printf("{ERROR: Can't assign to non-deferred word with inappropiate size %u}\n", m[assigned_word]);
}
MAKEPRIM(defer) {
    CONSUMER(' ', C_LOR(), , WARNING(edfer));
    makeWord(c, m, lorig, w_size, 0, 0, 0, CA(t_end, t_end), 2);
}
MAKEPRIM(postpone) {
    CONSUMER(' ', C_LOR(), , WARNING(postpone));
    Cell found_word = findWord(c, m, 'n', lorig, w_size);
    appendWord(c, m, CA(found_word), 1);
}
MAKEPRIM(save_input){
    R_SAVE();
    funcPush(c, m, c->inter_max-m);
    funcPush(c, m, c->inter_min-m);
    funcPush(c, m, c->inter_str-m);
    R_RESTORE();
}

MAKEPRIM(restore_input){
    R_SAVE();
    c->inter_str = m+funcPop(c, m);
    c->inter_min = m+funcPop(c, m);
    c->inter_max = m+funcPop(c, m);
    R_RESTORE();
}

// execute takes care of restoring the program counter for us
MAKEPRIM(interpret) {
    Cell s = dataPop(c, m), adr = dataPop(c, m);
    interpret(c, m, &m[adr], s, 0);
}

/*Interface to Offset-based allocator in oa.h*/
/*---------------------------------------------*/
MAKEPRIM(heap_init){ OA_init(m+c->heap_start, 0x2000); }
MAKEPRIM(defrag){ OA_defrag(); }
MAKEPRIM(allocate){
    Cell mem = OA_malloc(dataPop(c, m));
    mem = (mem == NIL) ? 0 : (mem + c->heap_start);
    dataPush(c, m, mem);
}
MAKEPRIM(free){ Cell mem = dataPop(c, m); if (mem == 0) return; OA_free(mem - c->heap_start); }

/* From here on it's all trivial boilerplate for C arithmetic operations and I/O*/
/*---------------------------------------------*/
#define INIT(_name) Cell _name = dataPop(c, m);
#define RINIT(_name) Cell _name = funcPop(c, m);
#define INIT_I(_name) int32_t _name = dataPop(c, m);
#define PUSH(...) dataPush(c, m, __VA_ARGS__)
#define RPUSH(...) funcPush(c, m, __VA_ARGS__)
#define OP_UN(_name, ...) MAKEPRIM(_name) { INIT_I(w1); PUSH(__VA_ARGS__); }
#define OP_BIN(_name, ...) MAKEPRIM(_name) { INIT_I(w2); INIT_I(w1); PUSH(__VA_ARGS__); }

#define ENABLE_HACK
#include "ugly_stackeffects.h"

// Eff/Reff interface: 'ni' is number of inputs (will be popped as w1, w2, w3... WHERE THE TOP OF THE STACK IS THE HIGHEST wN)
// It defines a primitive that takes that many inputs from the start, pushes as many outputs as defined in 'no',
// and then you can list the order (WHERE RIGHTMOST=TOP OF STACK) you want them to be pushed in.
// You can put any expression in the output, really
#define EFF(ni, no, _name, ...) EFF_##ni(_name, ;, ;, P_##no(__VA_ARGS__))
#define REFF(ni, no, _name, ...) REFF_##ni(_name, R_SAVE(), R_RESTORE(), RP_##no(__VA_ARGS__))


OP_BIN(add,    w1+w2);
OP_BIN(minus,  w1-w2);
OP_BIN(mult,   w1*w2);
OP_BIN(div,    w1/w2);
OP_BIN(mod,    w1%w2);
OP_BIN(rshift, w1>>w2);
OP_BIN(lshift, w1<<w2);
OP_BIN(max,    (w1>w2) ? w1 : w2);
OP_BIN(min,    (w1<w2) ? w1 : w2);
OP_BIN(and,    w1&w2);
OP_BIN(or,     w1|w2);
OP_BIN(xor,    w1^w2);
OP_BIN(eq,     BOOL(w1==w2));
OP_BIN(neq,    BOOL(w1!=w2));
OP_BIN(le,     BOOL(w1<w2));
OP_BIN(leq,    BOOL(w1<=w2));
OP_BIN(gr,     BOOL(w1>w2));
OP_BIN(geq,    BOOL(w1>=w2));

OP_UN(abs,         (w1 < 0) ? -w1 : w1);
OP_UN(arith_not,   -w1);
OP_UN(bitwise_not, ~w1);
OP_UN(logical_not,  w1 ? BOOL(0) : BOOL(1));

// Any possible warnings are just unused vars

/*Simple data stack manipulation*/
EFF(1, 0, drop) // ( w -- )
EFF(2, 1, nip, w2) // ( w1 w2 -- w2)
EFF(1, 2, dup, w1, w1) // ( w -- w w)
EFF(2, 3, over, w1, w2, w1) // ( w1 w2 -- w1 w2 w1)
EFF(2, 2, swap, w2, w1) // ( w1 w2 -- w2 w1)
EFF(3, 3, rot, w2, w3, w1) // ( w1 w2 w3 -- w2 w3 w1)
EFF(2, 3, tuck, w2, w1, w2) // ( w1 w2 -- w2 w1 w2)
EFF(2, 0, 2drop) // ( w1 w2 -- )
EFF(4, 2, 2nip, w3, w4) // ( w1 w2 w3 w4 -- w3 w4)
EFF(2, 4, 2dup, w1, w2, w1, w2) // ( w1 w2 -- w1 w2 w1 w2)
EFF(4, 6, 2over, w1, w2, w3, w4, w1, w2) // ( w1 w2 w3 w4 -- w1 w2 w3 w4 w1 w2)
EFF(4, 4, 2swap, w3, w4, w1, w2) // ( w1 w2 w3 w4 -- w3 w4 w1 w2)
EFF(6, 6, 2rot, w3, w4, w5, w6, w1, w2) // ( w1 w2 w3 w4 w5 w6 -- w3 w4 w5 w6 w1 w2)
EFF(4, 6, 2tuck, w3, w4, w1, w2, w3, w4) // ( w1 w2 w3 w4 -- w3 w4 w1 w2 w3 w4)

/* Return stack manipulation */
REFF(1, 0, rdrop) // R( w -- )
REFF(2, 1, rnip, w2) // R( w1 w2 -- w2)
REFF(1, 2, rdup, w1, w1) // R( w -- w w)
REFF(2, 3, rover, w1, w2, w1) // R( w1 w2 -- w1 w2 w1)
REFF(2, 2, rswap, w2, w1) // R( w1 w2 -- w2 w1)
REFF(3, 3, rrot, w2, w3, w1) // R( w1 w2 w3 -- w2 w3 w1)
REFF(2, 3, rtuck, w2, w1, w2) // R( w1 w2 -- w2 w1 w2)

/* Memory manipulation, returnstack-datastack interaction */
/*---------------------------------------------*/
MAKEPRIM(fetch) { dataPush(c, m, m[dataPop(c, m)]); }
MAKEPRIM(store) { Cell adr = dataPop(c, m), val = dataPop(c, m); m[adr] = val; }
MAKEPRIM(rget) { R_SAVE(); funcPush(c, m, dataPop(c, m)); R_RESTORE(); }
MAKEPRIM(rsend) { R_SAVE(); dataPush(c, m, funcPop(c, m)); R_RESTORE(); }
MAKEPRIM(2fetch) { Cell adr = dataPop(c, m); dataPush(c, m, m[adr]); dataPush(c, m, m[adr+1]); }
MAKEPRIM(2store) {
    Cell adr = dataPop(c, m), val2 = dataPop(c, m), val1 = dataPop(c, m);
    // Yes, they are stored in reverse according to the standard
    m[adr] = val2;
    m[adr+1] = val1;
}
MAKEPRIM(2rget) {
    R_SAVE();
    Cell w2 = dataPop(c, m), w1 = dataPop(c, m);
    funcPush(c, m, w1); funcPush(c, m, w2);
    R_RESTORE();
}
MAKEPRIM(2rsend) {
    R_SAVE();
    Cell w2 = funcPop(c, m), w1 = funcPop(c, m);
    dataPush(c, m, w1); dataPush(c, m, w2);
    R_RESTORE();
}

/*Printing and reading strings, numbers, memory, etc.*/
/*---------------------------------------------*/
MAKEPRIM(getchar){
    fflush(stdout);
    uint8_t w1 = getchar();
    if (w1 == '\n') w1 = 0;
    dataPush(c, m, w1);
    if (w1 == '\n') return;
    else while (getchar() != '\n');
}
MAKEPRIM(getnum){
    fflush(stdout);
    Cell w1;
    _Bool found = scanf("%d", &w1);
    while (getchar() != '\n');
    if (found) dataPush(c, m, w1);
    else       dataPush(c, m, -1);
}
MAKEPRIM(accept){
    fflush(stdout);
    Cell size_limit = dataPop(c, m);
    Cell address = dataPop(c, m);
    char tmp_str[size_limit];
    unsigned i;
    for (i = 0; i < size_limit; i++){
        char c = getchar();
        if (c == '\n') break;
        else tmp_str[i] = c;
    }
    if (i >= size_limit) /*Read and discard until newline*/
        for (int j = i; ; j++) { if (getchar() == '\n') break; }
    for (unsigned k = 0; k < i; k++) {
        m[address+k] = tmp_str[k];
    }
    dataPush(c, m, i);
}
MAKEPRIM(flushoutput){
    fflush(stdout);
}
MAKEPRIM(emit){
    unsigned char ch = dataPop(c, m);
    if (ch >= 0x20 && ch <= 0x7E) printf("%c", ch);
    else if (ch == 0x0A)          printf("%c", ch);
    else                          printf("[0x%X]", ch);
}
MAKEPRIM(type){
    Cell size = dataPop(c, m), str = dataPop(c, m);
    if (size == 0) printf("[EMPTY_STR]");
    for (unsigned i = 0; i < size; i++) {
        printf("%c", (unsigned char)m[str+i]);
    }
}
MAKEPRIM(cr) { printf("\n"); }
MAKEPRIM(spaces) {
    Cell x = dataPop(c, m);
    for (unsigned i = 0; i < x; i++) {
        printf(" ");
    }
}
MAKEPRIM(dot) { printf(" %d", dataPop(c, m)); }
MAKEPRIM(udot) { printf(" %u", dataPop(c, m)); }
MAKEPRIM(xdot) { printf(" %08X", dataPop(c, m)); }
MAKEPRIM(dotmem) {
    Cell start = dataPop(c, m), end = dataPop(c, m), step = dataPop(c, m);
    printMemory(m, start, end, step);
}
MAKEPRIM(dotstack) {
    unsigned stacksize = m[c->dstack_ptr]-(MEM_START(c->dstack_ptr));
    printf(" | s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %d", m[MEM_START(c->dstack_ptr)+i]);
    }
    printf(" |");
}
MAKEPRIM(udotstack) {
    unsigned stacksize = m[c->dstack_ptr]-(MEM_START(c->dstack_ptr));
    printf(" | s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %u", m[MEM_START(c->dstack_ptr)+i]);
    }
    printf(" |");
}
MAKEPRIM(dotstackret) {
    R_SAVE();
    unsigned stacksize = m[c->fstack_ptr]-(MEM_START(c->fstack_ptr));
    printf(" R| s <%d>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %d", m[MEM_START(c->fstack_ptr)+i]);
    }
    printf(" |");
    R_RESTORE();
}
MAKEPRIM(udotstackret) {
    unsigned stacksize = m[c->fstack_ptr]-(MEM_START(c->fstack_ptr));
    printf(" R| s <%u>:", stacksize);
    for (unsigned i = 0; i < stacksize; i++) {
        printf(" %u", m[MEM_START(c->fstack_ptr)+i]);
    }
    printf(" |");
}

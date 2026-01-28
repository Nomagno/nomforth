// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "forth.h"

static unsigned cell_strlen(Cell *i) { Cell *s; for (s = i; *s; ++s){}; return (s - i); }
static unsigned char_strlen(char *i) { char *s; for (s = i; *s; ++s){}; return (s - i); }

#define SCHECK(__var, __message) if (m[__var] == MEM_START(__var)) { printf("{%s stack underflow. ABORTING", __message); exit(1); }

void dataPush(Ctx *c, Cell *m, Cell v) { m[m[c->dstack_ptr]] = v;  m[c->dstack_ptr] += 1; }
void funcPush(Ctx *c, Cell *m, Cell v) { m[m[c->fstack_ptr]] = v;  m[c->fstack_ptr] += 1; }
Cell dataPop(Ctx *c, Cell *m)  { SCHECK(c->dstack_ptr, "Data");    m[c->dstack_ptr] -= 1;  return m[m[c->dstack_ptr]]; }
Cell funcPop(Ctx *c, Cell *m)  { SCHECK(c->fstack_ptr, "Return");  m[c->fstack_ptr] -= 1;  return m[m[c->fstack_ptr]]; }
Cell dataPeek(Ctx *c, Cell *m) { SCHECK(c->dstack_ptr, "Data");                            return m[m[c->dstack_ptr]-1]; }
Cell funcPeek(Ctx *c, Cell *m) { SCHECK(c->fstack_ptr, "Return");                          return m[m[c->fstack_ptr]-1]; }

Cell addToPad(Ctx *c, Cell *m, Cell *s, unsigned name_size) {
    if (s == NULL || *s == '\0')
        return 0;
    Cell str_loc = m[c->pad_pos_ptr];
    Cell str_size = name_size+1;
    m[str_loc]=str_size;
    for (unsigned i = 0; i < str_size-1; i++)
        m[str_loc+1+i]=s[i];
    m[c->pad_pos_ptr] = str_loc + str_size;
    return str_loc;
}

#define SET_VARIABLE(__x) (__x << 31)
#define SET_NO_TCO(__x) (__x << 30)
#define SET_NO_WARN(__x) (__x << 29)
#define SET_IMM(__x) (__x << 28)
#define CHECK_VARIABLE(__x) ((__x >> 31) & 1)
#define CHECK_NO_TCO(__x) ((__x >> 30) & 1)
#define CHECK_NO_WARN(__x) ((__x >> 29) & 1)
#define CHECK_IMM(__x) ((__x >> 28) & 1)

#define LPAREN (
#define RPAREN )
#define EXTRACT_SIZE(__x) (3+(m[__x+2] & 0x0000FFFF))
#define GET_PREV(__x, ...) (m[__x] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_NAME(__x, ...) (m[__x+1] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_HEADER(__x, ...) (m[__x+2] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_DATA(__x, ...) (m[__x + 3 __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN)])
#define COMPILE_STATE (m[c->compile_state_ptr])
#define DICTPTR (m[c->dict_pos_ptr])
#define PROGRAM_COUNTER (m[c->program_counter_ptr])
#define EXP_PTR m[c->exp_ptr]
#define BASE_PTR m[c->base_ptr]


void makeWord(Ctx *c, Cell *m, Cell *name, unsigned name_size,
              _Bool p, _Bool forbid_tco, _Bool allow_interpret, Cell *data, unsigned data_size) {
    Cell prev = DICTPTR;
    Cell prev_size = (prev == 0) ? DICT_START : EXTRACT_SIZE(prev);
    DICTPTR = prev + prev_size;

    Cell curr = DICTPTR;
    GET_PREV(curr) = prev;
    GET_NAME(curr) = addToPad(c, m, name, name_size);
    GET_HEADER(curr) = SET_NO_TCO(forbid_tco) | SET_NO_WARN(allow_interpret) | SET_IMM(p) | data_size;
    for (unsigned i = 0; i < data_size; i++)
        GET_DATA(curr, i) = data[i];
}
Cell appendWord(Ctx *c, Cell *m, Cell *data, unsigned data_size) {
    Cell curr = DICTPTR;
    unsigned size = EXTRACT_SIZE(curr);
    for (unsigned i = 0; i < data_size; i++)
        m[curr+size+i] = data[i];
    GET_HEADER(curr) += data_size;

    // Pointer to the last written byte of data,
    // used for easily placing references in the data
    // stack during the compilation process
    return (curr + (size-1) + data_size);
}

// Just to avoid pasting the same code twice
#define INSTANCE_FIND_WORD(__x) \
__x *n = (__x *)s;\
if (s_size == 0) return 0;\
_Bool condition = 1;\
Cell curr = DICTPTR;\
while (condition) {\
    Cell temp_str = GET_NAME(curr);\
    Cell temp_str_size = m[temp_str] - 1;\
    unsigned i;\
    _Bool are_equal = 1;\
    if (s_size != temp_str_size) are_equal = 0;\
    for (i = 0; i < s_size && are_equal; i++) {\
        if (toupper(m[temp_str+1+i]) != toupper((__x)n[i])) are_equal = 0;\
    }\
    if (are_equal && (i == s_size)) condition = 0;\
    else {\
        if (GET_PREV(curr) == 0) return 0;\
        else curr = GET_PREV(curr);\
    }\
}\
return curr;


Cell findWord(Ctx *c, Cell *m, char strtype, void *s, unsigned s_size) {
    if (strtype == 'n') { INSTANCE_FIND_WORD(Cell); } //native string
    else if (strtype == 'c') { INSTANCE_FIND_WORD(char); } //character/C string
    else { return 0; } //unknown
}


static int findChar(char x, char *str) {
    for (unsigned i = 0; str[i] != '\0'; i++)
        if (str[i] == x) return i;
    return -1;
}

static int delChar(char x, char *str) {
    unsigned count = 0;
    int pos = 0;
    while (findChar(x, str) != -1) {
        pos = findChar(x, str);
        count += 1;
        unsigned i;
        for (i = pos; str[i+1] != '\0'; i++)
            str[i] = str[i+1];
        str[i] = '\0';
    }
    if (count > 0) return pos;
    else if (count == 0) return -1;
    else return (-pos-1);
}

void executePrimitive(Ctx *c, Cell *m, Cell id) {
    if (primTable[id].func == NULL) printf("{WARNING: Undefined primitive 0x%X}\n", id);
    else primTable[id].func(c, m);
}
void executeWord(Ctx *c, Cell *m, Cell w) { 
    PROGRAM_COUNTER = w+3;
    //_Bool silent = m[c->flags_ptr] >> 1 & 1;
    _Bool trace  = m[c->flags_ptr] >> 2 & 1;
    _Bool reached_end = 0;
    _Bool do_trace = trace;

    if (do_trace) {
        printf("{TRACE: entering word 0x%X, ", w);
        if (GET_NAME(w) == 0) {
            printf("[[NO NAME]]");
        } else {
            for (unsigned i = 0; i < m[GET_NAME(w)]-1; i++) {
                printf("%c", m[GET_NAME(w)+1+i]);
            }
        }
        printf("}\n");
    }

    while (!reached_end) {
        Cell contents = PROGRAM_COUNTER;
        switch(m[contents]){
        case t_unknown_label:
            printf("{ERROR: 0 IS NOT A VALID INSTRUCTION, PC 0x%X}\n", contents);
            reached_end = 1;
            break;
        case t_nop:
            PROGRAM_COUNTER += 1;
            break;
        case t_primitive: ;
            if (do_trace) printf("{TRACE: going to primitive 0x%X}\n", m[contents+1]);

            // in case the primitive tries to (or accidentally) hijack the program
            // counter, we preserve and restore it ourselves
            Cell tmp_pc = PROGRAM_COUNTER;
            executePrimitive(c, m, m[contents+1]);
            PROGRAM_COUNTER = tmp_pc;

            PROGRAM_COUNTER += 2;
            break;
        case t_num:
            dataPush(c, m, m[contents+1]);
            PROGRAM_COUNTER += 2;
            break;
        case t_reljump:
            PROGRAM_COUNTER += m[contents+1];
            break;
        case t_condreljump: {
            Cell v = dataPop(c, m);
            if (v == 0) PROGRAM_COUNTER += m[contents+1];
            else PROGRAM_COUNTER += 2;
            }
            break;
        case t_reljumpback:
            PROGRAM_COUNTER -= m[contents+1];
            break;
        case t_condreljumpback: {
            Cell v = dataPop(c, m);
            if (v == 0) PROGRAM_COUNTER -= m[contents+1];
            else PROGRAM_COUNTER += 2;
            }
            break;
        case t_absjump:
            PROGRAM_COUNTER = m[contents+1];
            break;
        case t_leavelabel:
            printf("{ERROR: Non-replaced leave label, this shouldn't have happened}\n");
            reached_end = 1;
            break;         
        case t_end_notailcall:
            // Internally, the possibility of tail calls generated by an end label
            // breaks DOES> code generation catastrophically. To fix this,
            // we have an 'end without tail call' that behaves identically,
            // but will not be detected by the tail call optimizer
        case t_end:
            ;
            Cell next_pc = funcPop(c, m);
            if (do_trace) printf("{TRACE: going back to pc 0x%X from 0x%X}\n", next_pc, contents);
            if (next_pc == 0) {
                // If the next program counter is 0,
                // we're going back to the interpreter
                PROGRAM_COUNTER = 0;
                reached_end = 1;
            } else {
                // By convention the program counter needs to be incremented from the callee
                PROGRAM_COUNTER = next_pc;
                PROGRAM_COUNTER += 1;
            }
            break;
        case t_execute:
            ;
            Cell new_w = 0;
            new_w = dataPop(c, m);
        default:
            if (CHECK_NO_TCO(GET_HEADER(m[contents])) || (m[contents+1] != t_end)) {
                funcPush(c, m, PROGRAM_COUNTER);
            } else {
                // If it's a tail call no need to push another stack frame,
                // Unless the word is forbidden from being Tail Call Optimized
                if (do_trace) printf("{TRACE: Next jump is in tail position}\n");
            }
            if (m[contents] != t_execute) new_w = m[contents]; 
            contents = new_w+3;
            PROGRAM_COUNTER = contents;
            if (do_trace) {
                printf("{TRACE: jumping to word 0x%X, ", new_w);
                if (GET_NAME(new_w) == 0) {
                    printf("[[NO NAME]]");
                } else {
                    for (unsigned i = 0; i < m[GET_NAME(new_w)]-1; i++) {
                        printf("%c", m[GET_NAME(new_w)+1+i]);
                    }
                }
                printf("}\n");
            }
            break;
        }
    }
}
int consumeWord(Cell **s, const Cell *max, unsigned char target, _Bool skip_leading) {
    if (skip_leading)
        while (**s == target)
            *s += 1; 
    else if (**s == ' ')
        *s += 1;
    Cell *orig = *s;

    while (**s != target && *s <= max)
        *s += 1;
    Cell *now = *s;

    if (now > max) return -(now-orig);
    else return (now-orig);
}
int interpret(Ctx *c, Cell *m, Cell *l, unsigned l_size, _Bool silent) {
    _Bool l_c = 1;
    _Bool was_there_error = 0;
    c->input_start = l;
    c->input_end = l+l_size-1;
    c->input = l;
    while(l_c) {
        int w_size = consumeWord(&c->input, c->input_end, ' ', 1);
        if (w_size < 0) { l_c = 0; w_size = -w_size; } else
        if (w_size == 0) break;

        Cell *lorig = c->input-w_size;

        Cell w = findWord(c, m, 'n', lorig, w_size);
        if (w != 0) {
            // - Highest bit is for auxiliary info,
            // - Second highest is for TCO permissions,
            // - Third highest is 0 if it is forbidden to interpret the word,
            // - Fourth highest is for immediacy info
            _Bool priority = CHECK_IMM(GET_HEADER(w));
            _Bool allow_interpreting = CHECK_NO_WARN(GET_HEADER(w));
            if (priority || COMPILE_STATE == 0) {
                if (priority && !allow_interpreting && COMPILE_STATE == 0) {
                    printf("{WARNING: SHOULD NOT INTERPRET COMPILE-ONLY WORD ");
                    for (int i = 0; i < w_size; i++)
                        printf("%c", lorig[i]);
                    printf("}\n");
                }

                // Mark that lets the inner interpreter know
                //  it's going back to the outer interpreter
                funcPush(c, m, 0);
                executeWord(c, m, w);
            } else {
                dataPush(c, m, w);
                PRIM(comma)(c, m);
            }
        } else {
            *c->input = '\0';
            char *endptr;

            char tmpstring[w_size+1];
            for (int i = 0; i < w_size; i++)
                tmpstring[i] = lorig[i];
            tmpstring[w_size] = '\0';
            int pos = delChar('.', tmpstring);
            _Bool valid_dot = (w_size-1);
            if (pos > 0) { //one dot
                if ((int)m[EXP_PTR] < 0) {
                    int number_of_digits_after_point = (w_size-1)-pos;
                    valid_dot = number_of_digits_after_point == -(int)m[EXP_PTR];
                } else {
                    valid_dot = 0;
                }
                w_size -= 1;
            } else if (pos == -1) { // no dots
                valid_dot = (int)m[EXP_PTR] >= 0;
            } else { //multiple dots, nonsense
                valid_dot = 0;
            }

            Cell val = strtol(tmpstring, &endptr, BASE_PTR);
            int converted_num_size = endptr-tmpstring;

            if (l_c != 0) *c->input = ' ';

            if (valid_dot && converted_num_size == w_size) {
                if (COMPILE_STATE == 0)
                    dataPush(c, m, val);
                else {
                    // Tag for literal
                    dataPush(c, m, t_num); PRIM(comma)(c, m);
                    // Actual value
                    dataPush(c, m, val); PRIM(comma)(c, m);
                }
            } else {
                if (valid_dot) printf("{ERROR: unknown word ");
                else printf("{ERROR: bad size after fixed point ");
                for (int i = 0; i < w_size + (pos > 0); i++)
                    printf("%c", lorig[i]);
                printf("}\n");
                l_c = 0;
                was_there_error = 1;
            }
        }
    }
    if (!was_there_error && !silent) printf(" {OK}\n");
    //if (was_there_error)
}

void init(Ctx *c, Cell *m) {
    c->base_ptr = BASE;
    c->exp_ptr = EXP;
    c->compile_state_ptr = CS;
    c->program_counter_ptr = PC;
    c->dstack_ptr = DSTACK_START-1;
    c->fstack_ptr = FSTACK_START-1;
    c->flags_ptr = FLAGS;
    c->dict_pos_ptr = DICT_START-1;
    c->pad_pos_ptr = PAD_START-1;
    c->heap_start = HEAP_START;
    c->inbuf_start = INBUF_START;

    m[c->base_ptr] = 10;
    m[c->exp_ptr] = 0;
    m[c->compile_state_ptr] = 0;
    m[c->program_counter_ptr] = 0;
    m[c->dstack_ptr] = DSTACK_START;
    m[c->fstack_ptr] = FSTACK_START;
    m[c->dict_pos_ptr] = NILPTR;
    m[c->pad_pos_ptr] = PAD_START;
    m[c->flags_ptr] = 0;

    initPrimitives(c, m);
}
void initPrimitives(Ctx *c, Cell *m) {
    for (unsigned i = 0; i < PRIM_NUM; i++) {
        if (primTable[i].func != NULL) {
            unsigned name_size = char_strlen(primTable[i].name);
            Cell tmpnatstring[name_size];
            for (unsigned j = 0; j < name_size; j++)
                tmpnatstring[j] = primTable[i].name[j];
            makeWord(c, m, tmpnatstring, name_size, primTable[i].priority,
                      primTable[i].forbid_tco, primTable[i].allow_interpret,
                      CA(t_primitive, i, t_end), 3);
        }
    }
}
void printMemory(Cell *m, unsigned start, unsigned increment, unsigned maxval) {
    for(unsigned i = start; i < maxval; i += increment) {
        printf("0x%4X:", i);
        for (unsigned j = i; j < i+increment; j++) {
            // Colour non-zero cells
            if (m[j] != 0) printf(" \033[30;41m%08X\033[0m", m[j]);
            else           printf(           " %08X",        m[j]);
        }
        printf("\n");
    }
}

void repl(Ctx *c, Cell *m) {
    char line[1024];
    _Bool silent = 0;
    _Bool quit = 0;
    while(1) {
        silent = m[c->flags_ptr] >> 1 & 1;
        quit = m[c->flags_ptr] >> 0 & 1;
        if (quit) return;

        fgets(line, sizeof(line), stdin);
        unsigned lsize = char_strlen(line)-1; // Exclude terminating newline
        if (char_strlen(line) <= 0)
            continue;

        for (unsigned i = 0; i < lsize; i++)
            m[c->inbuf_start+i] = line[i];
        m[c->inbuf_start+lsize] = '\0';
        if (!silent) printf("OUTPUT:");
        interpret(c, m, &m[c->inbuf_start], lsize, silent);
    }    
}

#ifndef NOMFORTH_LIB
Cell memory[MEM_MAX];
Ctx  context;
int main(void) {
    init(&context, memory);
    printf("Welcome to nomForth!\nTo exit, type QUIT.\n");
    repl(&context, memory);
    printf("\nThanks for using nomForth.\n");
}
#endif

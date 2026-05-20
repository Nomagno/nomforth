

// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "forth.h"

static unsigned cell_strlen(Cell *i) { Cell *s; for (s = i; *s; ++s){}; return (s - i); }
static unsigned char_strlen(char *i) { char *s; for (s = i; *s; ++s){}; return (s - i); }

#define SCHECK(__var, __message) if (c->m[__var] == MEM_START(__var)) { printf("{%s stack underflow. ABORTING}", __message); exit(1); }

void dataPush(Ctx *c, Cell v) { c->m[c->m[c->dstack_ptr]] = v;  c->m[c->dstack_ptr] += 1; }
void funcPush(Ctx *c, Cell v) { c->m[c->m[c->fstack_ptr]] = v;  c->m[c->fstack_ptr] += 1; }
Cell dataPop(Ctx *c)  { SCHECK(c->dstack_ptr, "Data");    c->m[c->dstack_ptr] -= 1;  return c->m[c->m[c->dstack_ptr]]; }
Cell funcPop(Ctx *c)  { SCHECK(c->fstack_ptr, "Return");  c->m[c->fstack_ptr] -= 1;  return c->m[c->m[c->fstack_ptr]]; }
Cell dataPeek(Ctx *c) { SCHECK(c->dstack_ptr, "Data");                            return c->m[c->m[c->dstack_ptr]-1]; }
Cell funcPeek(Ctx *c) { SCHECK(c->fstack_ptr, "Return");                          return c->m[c->m[c->fstack_ptr]-1]; }

Cell addToPad(Ctx *c, Cell *s, unsigned name_size) {
    if (s == NULL || *s == '\0')
        return 0;
    Cell str_loc = c->m[c->pad_pos_ptr];
    Cell str_size = name_size+1;
    c->m[str_loc]=str_size;
    for (unsigned i = 0; i < str_size-1; i++)
        c->m[str_loc+1+i]=s[i];
    c->m[c->pad_pos_ptr] = str_loc + str_size;
    return str_loc;
}

void makeWord(Ctx *c, Cell *name, unsigned name_size,
              _Bool p, _Bool forbid_tco, _Bool allow_interpret, Cell *data, unsigned data_size) {
    Cell prev = DICTPTR;
    Cell prev_size = (prev == 0) ? DICT_START : EXTRACT_SIZE(prev);
    DICTPTR = prev + prev_size;

    Cell curr = DICTPTR;
    GET_PREV(curr) = prev;
    GET_NAME(curr) = addToPad(c, name, name_size);
    GET_HEADER(curr) = SET_NO_TCO(forbid_tco) | SET_NO_WARN(allow_interpret) | SET_IMM(p) | data_size;
    for (unsigned i = 0; i < data_size; i++)
        GET_DATA(curr, i) = data[i];
}
Cell appendWord(Ctx *c, Cell *data, unsigned data_size) {
    Cell curr = DICTPTR;
    unsigned size = EXTRACT_SIZE(curr);
    for (unsigned i = 0; i < data_size; i++)
        c->m[curr+size+i] = data[i];
    GET_HEADER(curr) += data_size;

    // Pointer to the last written byte of data,
    // used for easily placing references in the data
    // stack during the compilation process
    return (curr + (size-1) + data_size);
}

// Just to avoid pasting the same code twice
#define INSTANCE_FIND_WORD(__type) \
__type *n = (__type *)s;\
if (s_size == 0) return 0;\
_Bool condition = 1;\
Cell curr = DICTPTR;\
while (condition) {\
    Cell temp_str = GET_NAME(curr);\
    if (temp_str == 0 || (temp_str & (1 << 31)) != 0) { goto skip_##__type; }\
    Cell temp_str_size = c->m[temp_str] - 1;\
    unsigned i;\
    _Bool are_equal = 1;\
    if (s_size != temp_str_size) are_equal = 0;\
    for (i = 0; i < s_size && are_equal; i++) {\
        if (toupper(c->m[temp_str+1+i]) != toupper((__type)n[i])) are_equal = 0;\
    }\
    if (are_equal && (i == s_size)) condition = 0;\
    else {\
        skip_##__type:\
        if (GET_PREV(curr) == 0) return 0;\
        else curr = GET_PREV(curr);\
    }\
}\
return curr;


Cell findWord(Ctx *c, char strtype, void *s, unsigned s_size) {
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

void executePrimitive(Ctx *c, Cell id) {
    if (primTable[id].func == NULL) printf("{WARNING: Undefined primitive 0x%X}\n", id);
    else primTable[id].func(c);
}
void executeWord(Ctx *c, Cell w) { 
    PROGRAM_COUNTER = w+3;
    //_Bool silent = c->m[c->flags_ptr] >> 1 & 1;
    _Bool trace  = c->m[c->flags_ptr] >> 2 & 1;
    _Bool reached_end = 0;
    _Bool do_trace = trace;

    if (do_trace) {
        printf("{TRACE: entering word 0x%X, ", w);
        if (GET_NAME(w) == 0) {
            printf("[[NO NAME]]");
        } else {
            for (unsigned i = 0; i < c->m[GET_NAME(w)]-1; i++) {
                printf("%c", c->m[GET_NAME(w)+1+i]);
            }
        }
        printf("}\n");
    }

    while (!reached_end) {
        Cell contents = PROGRAM_COUNTER;
        switch(c->m[contents]){
        case t_unknown_label:
            printf("{ERROR: 0 IS NOT A VALID INSTRUCTION, PC 0x%X}\n", contents);
            reached_end = 1;
            break;
        case t_nop:
            PROGRAM_COUNTER += 1;
            break;
        case t_primitive: ;
            if (do_trace) printf("{TRACE: going to primitive 0x%X}\n", c->m[contents+1]);

            // in case the primitive tries to (or accidentally) hijack the program
            // counter, we preserve and restore it ourselves
            Cell tmp_pc = PROGRAM_COUNTER;
            executePrimitive(c, c->m[contents+1]);
            PROGRAM_COUNTER = tmp_pc;

            PROGRAM_COUNTER += 2;
            break;
        case t_num:
            dataPush(c, c->m[contents+1]);
            PROGRAM_COUNTER += 2;
            break;
        case t_reljump:
            PROGRAM_COUNTER += c->m[contents+1];
            break;
        case t_condreljump: {
            Cell v = dataPop(c);
            if (v == 0) PROGRAM_COUNTER += c->m[contents+1];
            else PROGRAM_COUNTER += 2;
            }
            break;
        case t_reljumpback:
            PROGRAM_COUNTER -= c->m[contents+1];
            break;
        case t_condreljumpback: {
            Cell v = dataPop(c);
            if (v == 0) PROGRAM_COUNTER -= c->m[contents+1];
            else PROGRAM_COUNTER += 2;
            }
            break;
        case t_absjump:
            PROGRAM_COUNTER = c->m[contents+1];
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
            Cell next_pc = funcPop(c);
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
            new_w = dataPop(c);
        default:
            if (CHECK_NO_TCO(GET_HEADER(c->m[contents])) || (c->m[contents+1] != t_end)) {
                funcPush(c, PROGRAM_COUNTER);
            } else {
                // If it's a tail call no need to push another stack frame,
                // Unless the word is forbidden from being Tail Call Optimized
                if (do_trace) printf("{TRACE: Next jump is in tail position}\n");
            }
            if (c->m[contents] != t_execute) new_w = c->m[contents]; 
            contents = new_w+3;
            PROGRAM_COUNTER = contents;
            if (do_trace) {
                printf("{TRACE: jumping to word 0x%X, ", new_w);
                if (GET_NAME(new_w) == 0) {
                    printf("[[NO NAME]]");
                } else {
                    for (unsigned i = 0; i < c->m[GET_NAME(new_w)]-1; i++) {
                        printf("%c", c->m[GET_NAME(new_w)+1+i]);
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
int interpret(Ctx *c, Cell *src_start, unsigned src_size, _Bool silent) {
    _Bool do_continue = 1;
    _Bool was_there_error = 0;
    c->input_start = src_start;
    c->input_end = c->input_start+src_size-1;
    c->input = src_start;
    while(do_continue) {
        int w_size = consumeWord(&c->input, c->input_end, ' ', 1);

        // This word is the last of the line, take absolute value to ake its length
        if (w_size < 0) { do_continue = 0; w_size = -w_size; }

        // No more characters available
        if (w_size == 0) break;

        Cell *lorig = c->input-w_size;

        Cell w = findWord(c, 'n', lorig, w_size);
        if (w != 0) {
            // - Highest bit is for auxiliary info,
            // - Second highest is for TCO permissions,
            // - Third highest is 0 if it is forbidden to interpret the word,
            // - Fourth highest is for immediacy info
            _Bool priority = CHECK_IMM(GET_HEADER(w));
            _Bool allow_interpreting = CHECK_NO_WARN(GET_HEADER(w));
            if (priority || COMPILE_STATE == 0) {
                if (priority && !allow_interpreting && COMPILE_STATE == 0) {
                    printf("{ERROR: CAN NOT INTERPRET COMPILE-ONLY WORD ");
                    for (int i = 0; i < w_size; i++)
                        printf("%c", lorig[i]);
                    printf("}\n");
                    do_continue = 0;
                    was_there_error = 1;
                } else {
                    // Mark that lets the inner interpreter know
                    //  it's going back to the outer interpreter
                    funcPush(c, 0);
                    executeWord(c, w);
                }
            } else {
                dataPush(c, w);
                PRIM(comma)(c);
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
                if ((int)c->m[EXP_PTR] < 0) {
                    int number_of_digits_after_point = (w_size-1)-pos;
                    valid_dot = number_of_digits_after_point == -(int)c->m[EXP_PTR];
                } else {
                    valid_dot = 0;
                }
                w_size -= 1;
            } else if (pos == -1) { // no dots
                valid_dot = (int)c->m[EXP_PTR] >= 0;
            } else { //multiple dots, nonsense
                valid_dot = 0;
            }

            Cell val = strtol(tmpstring, &endptr, BASE_PTR);
            int converted_num_size = endptr-tmpstring;

            if (do_continue != 0) *c->input = ' ';

            if (valid_dot && converted_num_size == w_size) {
                if (COMPILE_STATE == 0)
                    dataPush(c, val);
                else {
                    // Tag for literal
                    dataPush(c, t_num); PRIM(comma)(c);
                    // Actual value
                    dataPush(c, val); PRIM(comma)(c);
                }
            } else {
                if (valid_dot) printf("{ERROR: unknown word ");
                else printf("{ERROR: bad size after fixed point ");
                for (int i = 0; i < w_size + (pos > 0); i++)
                    printf("%c", lorig[i]);
                printf("}\n");
                do_continue = 0;
                was_there_error = 1;
            }
        }
    }
    if (!was_there_error && !silent) printf(" {OK}\n");
    //if (was_there_error)
}

void init(Ctx *c) {
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

    c->m = malloc(sizeof(Cell)*MEM_MAX);

    c->m[c->base_ptr] = 10;
    c->m[c->exp_ptr] = 0;
    c->m[c->compile_state_ptr] = 0;
    c->m[c->program_counter_ptr] = 0;
    c->m[c->dstack_ptr] = DSTACK_START;
    c->m[c->fstack_ptr] = FSTACK_START;
    c->m[c->dict_pos_ptr] = NILPTR;
    c->m[c->pad_pos_ptr] = PAD_START;
    c->m[c->flags_ptr] = 0;

    initPrimitives(c);
}
void initPrimitives(Ctx *c) {
    for (unsigned i = 0; i < PRIM_NUM; i++) {
        if (primTable[i].func != NULL) {
            unsigned name_size = char_strlen(primTable[i].name);
            Cell tmpnatstring[name_size];
            for (unsigned j = 0; j < name_size; j++)
                tmpnatstring[j] = primTable[i].name[j];
            makeWord(c, tmpnatstring, name_size, primTable[i].priority,
                      primTable[i].forbid_tco, primTable[i].allow_interpret,
                      CA(t_primitive, i, t_end), 3);
        }
    }
}

void repl(Ctx *c) {
    char line[1024];
    _Bool silent = 0;
    _Bool quit = 0;
    while(1) {
        silent = c->m[c->flags_ptr] >> 1 & 1;
        quit = c->m[c->flags_ptr] >> 0 & 1;
        if (quit) return;

        fgets(line, sizeof(line), stdin);
        unsigned lsize = char_strlen(line)-1; // Exclude terminating newline
        if (char_strlen(line) <= 0)
            continue;

        for (unsigned i = 0; i < lsize; i++)
            c->m[c->inbuf_start+i] = line[i];
        c->m[c->inbuf_start+lsize] = '\0';
        if (!silent) printf("OUTPUT:");
        interpret(c, &c->m[c->inbuf_start], lsize, silent);
    }    
}

#ifndef NOMFORTH_LIB
Ctx  context;
int main(void) {
    init(&context);
    printf("Welcome to nomForth!\nTo exit, type QUIT.\n");
    repl(&context);
    printf("\nThanks for using nomForth.\n");
}
#endif

// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "forth.h"

unsigned cell_strlen(Cell *i) { Cell *s; for (s = i; *s; ++s){}; return (s - i); }
unsigned char_strlen(char *i) { char *s; for (s = i; *s; ++s){}; return (s - i); }
void cell_char_memcpy(Cell *dest, const char *src, unsigned len) {
    for (unsigned i = 0; i < len; i++)
        dest[i] = src[i];
}
void char_cell_memcpy(char *dest, const Cell *src, unsigned len) {
    for (unsigned i = 0; i < len; i++)
        dest[i] = src[i];
}

void nf_memcpy(void *dest, const void *src, unsigned len) {
    for (unsigned i = 0; i < len; i++)
        ((char *)dest)[i] = ((char *)src)[i];
}

#define NOMFORTH_SYSTEM_MESSAGE(_prepended_string, _appended_statement)\
{ printf("{"); printf(_prepended_string); _appended_statement; printf("}\n"); }

#define SCHECK(__var, __message) if (c->m[__var] == MEM_START(__var)) { printf("{%s stack underflow. ABORTING}", __message); exit(1); }

void dataPush(Ctx *c, Cell v) { c->m[c->m[c->dstack_ptr]] = v;  c->m[c->dstack_ptr] += 1; }
void funcPush(Ctx *c, Cell v) { c->m[c->m[c->fstack_ptr]] = v;  c->m[c->fstack_ptr] += 1; }
Cell dataPop(Ctx *c)  { SCHECK(c->dstack_ptr, "Data");    c->m[c->dstack_ptr] -= 1;  return c->m[c->m[c->dstack_ptr]]; }
Cell funcPop(Ctx *c)  { SCHECK(c->fstack_ptr, "Return");  c->m[c->fstack_ptr] -= 1;  return c->m[c->m[c->fstack_ptr]]; }
Cell dataPeek(Ctx *c) { SCHECK(c->dstack_ptr, "Data");                            return c->m[c->m[c->dstack_ptr]-1]; }
Cell funcPeek(Ctx *c) { SCHECK(c->fstack_ptr, "Return");                          return c->m[c->m[c->fstack_ptr]-1]; }

Cell addToYarnball(Ctx *c, Cell *s, unsigned name_size) {
    if (s == NULL || *s == '\0')
        return 0;
    Cell str_loc = c->m[c->yarnball_pos_ptr];
    c->m[str_loc]=name_size+1;
    nf_memcpy(&c->m[str_loc+1], s, name_size*sizeof(Cell));
    c->m[c->yarnball_pos_ptr] = str_loc + name_size + 1;
    return str_loc;
}

void makeWord(Ctx *c, Cell *name, unsigned name_size,
              _Bool p, _Bool forbid_tco, _Bool allow_interpret, Cell *data, unsigned data_size) {
    Cell prev = DICTPTR;
    Cell prev_size = (prev == 0) ? DICT_START : EXTRACT_SIZE(prev);
    DICTPTR = prev + prev_size;

    Cell curr = DICTPTR;
    GET_PREV(curr) = prev;
    GET_NAME(curr) = addToYarnball(c, name, name_size);
    GET_HEADER(curr) = SET_NO_TCO(forbid_tco) | SET_NO_WARN(allow_interpret) | SET_IMM(p) | data_size;
    nf_memcpy(&GET_DATA(curr), data, data_size*sizeof(Cell));
}
Cell appendWord(Ctx *c, Cell *data, unsigned data_size) {
    Cell curr = DICTPTR;
    unsigned size = EXTRACT_SIZE(curr);
    nf_memcpy(&c->m[curr+size], data, data_size*sizeof(Cell));
    GET_HEADER(curr) += data_size;

    // Pointer to the last written byte of data,
    // used for easily placing references in the data
    // stack during the compilation process
    return (curr + (size-1) + data_size);
}

Cell findWord(Ctx *c, const Cell *s, unsigned s_size) {
    if (s_size == 0) return 0;
    _Bool condition = 1;
    Cell curr = DICTPTR;\
    while (condition) {
        Cell temp_str = GET_NAME(curr);
        if (temp_str == 0 || (temp_str & ((unsigned)1 << 31)) != 0) { goto skip; }
        Cell temp_str_size = c->m[temp_str] - 1;
        unsigned i;
        _Bool are_equal = 1;
        if (s_size != temp_str_size) are_equal = 0;
        for (i = 0; i < s_size && are_equal; i++) {
            if (toupper(c->m[temp_str+1+i]) != toupper(s[i])) are_equal = 0;
        }
        if (are_equal && (i == s_size)) condition = 0;
        else {
            skip:
            if (GET_PREV(curr) == 0) return 0;
            else curr = GET_PREV(curr);
        }
    }
    return curr;
}


// First occurence of x in str, or -1 if it does not occur
static int findChar(char x, char *str) {
    for (unsigned i = 0; str[i] != '\0'; i++)
        if (str[i] == x) return i;
    return -1;
}

// Removes first occurence of x from str
// Returns position of deletal, or -1 if not found
static int delChar(char x, char *str) {
    int pos = findChar(x, str);
    if (pos == -1) {
        return -1;
    } else {
        unsigned i;
        for (i = pos; str[i+1] != '\0'; i++)
            str[i] = str[i+1];
        str[i] = '\0';
        return pos;
    }
}

void executePrimitive(Ctx *c, Cell id) {
    if (primTable[id].func == NULL) printf("{WARNING: Undefined primitive 0x%X}\n", id);
    else primTable[id].func(c);
}

void executeWord(Ctx *c, Cell w) {
    PROGRAM_COUNTER = w+3;

    if (w < c->dict_pos_ptr+1) {
        NOMFORTH_SYSTEM_MESSAGE("ERROR: INVALID XT ADDRESS (TOO LOW): ",
                                printf("0x%X", w));
        return;
    }

    //_Bool silent = c->m[c->flags_ptr] >> 1 & 1;
    _Bool trace  = c->m[c->flags_ptr] >> 2 & 1;
    _Bool reached_end = 0;
    _Bool do_trace = trace;

    if (do_trace)
        NOMFORTH_SYSTEM_MESSAGE("TRACE: jumping to word ",
                    printf("0x%X, ", w);
                    if (GET_NAME(w) == 0) {
                        printf("[[NO NAME]]");
                    } else {
                        unsigned str_size = c->m[GET_NAME(w)] - 1;
                        Cell *str_start = &c->m[GET_NAME(w)+1];
                        PRINT_CELL_STRING(str_start, str_size);
                    });

    while (!reached_end) {
        Cell contents = PROGRAM_COUNTER;
        switch(c->m[contents]){
        case t_unknown_label:
            NOMFORTH_SYSTEM_MESSAGE("ERROR: 0 IS NOT A VALID INSTRUCTION, PC ",
                                    printf("0x%X", contents));
            reached_end = 1;
            break;
        case t_nop:
            PROGRAM_COUNTER += 1;
            break;
        case t_primitive: ;
            if (do_trace)
                NOMFORTH_SYSTEM_MESSAGE("TRACE: going to primitive ",
                                        printf("0x%X", c->m[contents+1]));

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
            NOMFORTH_SYSTEM_MESSAGE("ERROR: Non-replaced leave label, this shouldn't have happened", );
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
            if (do_trace)
                NOMFORTH_SYSTEM_MESSAGE("TRACE: going back to pc ",
                                        printf("0x%X from 0x%X", next_pc, contents));

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
            if (c->m[contents] != t_execute)
                new_w = c->m[contents];

            if (new_w < c->dict_pos_ptr+1) {
                NOMFORTH_SYSTEM_MESSAGE("ERROR: INVALID XT ADDRESS (TOO LOW): ",
                                        printf("0x%X", new_w));
                reached_end = 1;
                break;
            }

            if (CHECK_NO_TCO(GET_HEADER(c->m[contents])) || (c->m[contents+1] != t_end)) {
                funcPush(c, PROGRAM_COUNTER);
            } else {
                // If it's a tail call no need to push another stack frame,
                // Unless the word is forbidden from being Tail Call Optimized
                if (do_trace)
                    NOMFORTH_SYSTEM_MESSAGE("TRACE: Next jump is in tail position", );
            }

            contents = new_w+3;
            PROGRAM_COUNTER = contents;
            if (do_trace) {
                NOMFORTH_SYSTEM_MESSAGE("TRACE: jumping to word ",
                            printf("0x%X, ", new_w);
                            if (GET_NAME(new_w) == 0) {
                                printf("[[NO NAME]]");
                            } else {
                                unsigned str_size = c->m[GET_NAME(new_w)] - 1;
                                Cell *str_start = &c->m[GET_NAME(new_w)+1];
                                PRINT_CELL_STRING(str_start, str_size);
                            });
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

_Bool parseNumber(unsigned base, unsigned exp, Cell *lorig, unsigned w_size, unsigned *retnum) {
    char tmpstring[w_size+1];
    char_cell_memcpy(tmpstring, lorig, w_size);
    tmpstring[w_size] = '\0';

    int pos = delChar('.', tmpstring);
    enum {Valid_Fixed_Point, Invalid_Fixed_Point, Valid_Integer, Invalid_Integer}
        number_validity = Valid_Integer;
    if (pos >= 0) { //dot found
        if ((int)exp < 0) {
            // Numbers can only contain dots if the current exponent of the base is negative
            int number_of_digits_after_point = (w_size-1)-pos;
            if (number_of_digits_after_point == -(int)exp) {
                number_validity = Valid_Fixed_Point;
            } else {
                number_validity = Invalid_Fixed_Point;
            }
        } else {
            // Else, it's not allowed to have dots
            number_validity = Invalid_Integer;
        }
    } else { // no dots
        // Dots are mandatory if the exponent of the base is 0 or positive
        if ((int)exp >= 0) {
            number_validity = Valid_Integer;
        } else {
            number_validity = Invalid_Fixed_Point;
        }
    }

    char *endptr;
    Cell val = strtol(tmpstring, &endptr, base);

    unsigned converted_num_size = endptr-tmpstring;
    _Bool valid = number_validity == Valid_Fixed_Point || number_validity == Valid_Integer;
    if (valid && converted_num_size == w_size - ((pos >= 0) ? 1 : 0)) {
        *retnum = val;
        return 1; // All okay
    } else {
        return 0; // Not okay
    }
}

int interpret(Ctx *c, Cell *src_start, unsigned src_size, _Bool silent) {
    _Bool do_continue = 1;
    _Bool was_there_error = 0;
    c->input_start = src_start;
    c->input_end = c->input_start+src_size-1;
    c->input = src_start;
    while(do_continue) {
        int w_size_raw = consumeWord(&c->input, c->input_end, ' ', 1);

        // This word is the last of the line, take absolute value to ake its length
        if (w_size_raw < 0) { do_continue = 0; w_size_raw = -w_size_raw; }
        unsigned w_size = w_size_raw;

        // No more characters available
        if (w_size == 0) break;

        Cell *lorig = c->input-w_size;

        Cell w = findWord(c, lorig, w_size);
        if (w != 0) {
            // - Highest bit is for auxiliary info,
            // - Second highest is for TCO permissions,
            // - Third highest is 0 if it is forbidden to interpret the word,
            // - Fourth highest is for immediacy info
            _Bool priority = CHECK_IMM(GET_HEADER(w));
            _Bool allow_interpreting = CHECK_NO_WARN(GET_HEADER(w));
            if (priority || COMPILE_STATE == 0) {
                if (priority && !allow_interpreting && COMPILE_STATE == 0) {
                    NOMFORTH_SYSTEM_MESSAGE("ERROR: CAN NOT INTERPRET COMPILE-ONLY WORD ",
                                            PRINT_CELL_STRING(lorig, w_size))
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

            unsigned parsed_num;
            _Bool is_number = parseNumber(BASE_VAL, EXP_VAL, lorig, w_size, &parsed_num);

            if (do_continue != 0) *c->input = ' ';

            if (is_number) {
                if (COMPILE_STATE == 0)
                    dataPush(c, parsed_num);
                else {
                    // Tag for literal
                    dataPush(c, t_num); PRIM(comma)(c);
                    // Actual value
                    dataPush(c, parsed_num); PRIM(comma)(c);
                }
            } else {
                NOMFORTH_SYSTEM_MESSAGE("ERROR: unknown word ",
                                        PRINT_CELL_STRING(lorig, w_size))
                do_continue = 0;
                was_there_error = 1;
            }
        }
    }
    if (!was_there_error && !silent) printf(" {OK}\n");
    return was_there_error;
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
    c->yarnball_pos_ptr = YARNBALL_START-1;
    c->heap_start = HEAP_START;
    c->inbuf_start = INBUF_START;

    c->m = malloc(MEM_MAX*sizeof(Cell));

    c->m[c->base_ptr] = 10;
    c->m[c->exp_ptr] = 0;
    c->m[c->compile_state_ptr] = 0;
    c->m[c->program_counter_ptr] = 0;
    c->m[c->dstack_ptr] = DSTACK_START;
    c->m[c->fstack_ptr] = FSTACK_START;
    c->m[c->dict_pos_ptr] = NILPTR;
    c->m[c->yarnball_pos_ptr] = YARNBALL_START;
    c->m[c->flags_ptr] = 0;

    initPrimitives(c);
    initConstantWords(c);
}
void initPrimitives(Ctx *c) {
    for (unsigned i = 0; i < PRIM_NUM; i++) {
        if (primTable[i].func != NULL) {
            unsigned name_size = char_strlen(primTable[i].name);
            Cell tmpnatstring[name_size];
            cell_char_memcpy(tmpnatstring, primTable[i].name, name_size);
            makeWord(c, tmpnatstring, name_size, primTable[i].priority,
                      primTable[i].forbid_tco, primTable[i].allow_interpret,
                      CA(t_primitive, i, t_end), 3);
        }
    }
}

#define ADD_LOADER(_str, _val)\
    tmpstring = _str;\
    name_size = char_strlen(_str);\
    tmpnatstring = malloc(name_size*sizeof(Cell));\
    for (unsigned i = 0; i < name_size; i++) tmpnatstring[i] = tmpstring[i]; \
    makeWord(c, tmpnatstring, name_size,\
            IMMEDIATE_WORD, PERM_DOESNOT_APPLY, DISALLOW_INTERPRET,\
            CA(t_num, t_num, comma_address, t_num, _val, comma_address, t_end), 7); /*equivalent to : [WORDNAME] [_val] LIT, ; immediate */\
    free(tmpnatstring);


void initConstantWords(Ctx *c) {
    char *tmpstring;
    Cell *tmpnatstring;
    unsigned name_size;

    const Cell comma_lit[] = {','};
    Cell comma_address = findWord(c, comma_lit, COUNTOF(comma_lit));

    ADD_LOADER("C_T_UNKNOWN", t_unknown_label);
    ADD_LOADER("C_T_NOP", t_nop);
    ADD_LOADER("C_T_PRIM", t_primitive);
    ADD_LOADER("C_T_NUM", t_num);
    ADD_LOADER("C_T_RJMP", t_reljump);
    ADD_LOADER("C_T_RBJMP", t_reljumpback);
    ADD_LOADER("C_T_CRJMP", t_condreljump);
    ADD_LOADER("C_T_CRBJMP", t_condreljumpback);
    ADD_LOADER("C_T_ABSJMP", t_absjump);
    ADD_LOADER("C_T_L", t_leavelabel);
    ADD_LOADER("C_T_E", t_end);
    ADD_LOADER("C_T_E_NTC", t_end_notailcall);
    ADD_LOADER("C_T_EXEC", t_execute);

    ADD_LOADER("C_PC_ADR", PC);
    ADD_LOADER("C_STATE_ADR", CS);
    ADD_LOADER("C_STRPOS_ADR", STRPOS);
    ADD_LOADER("C_STRSIZE_ADR", STRSIZE);
    ADD_LOADER("C_STRPTR_ADR", STRPTR);
    ADD_LOADER("C_FLAGS_ADR", FLAGS);
    ADD_LOADER("C_BASE_ADR", BASE);
    ADD_LOADER("C_EXP_ADR", EXP);
    ADD_LOADER("C_DICT_ADR", DICT_START-1);
    ADD_LOADER("C_STACK_ADR", DSTACK_START-1);
    ADD_LOADER("C_RSTACK_ADR", FSTACK_START-1);
    ADD_LOADER("C_USERMEM_ADR", USERMEM_START);
    ADD_LOADER("C_HEAP_ADR", HEAP_START);
    ADD_LOADER("C_YARNBALL_ADR", YARNBALL_START);
    ADD_LOADER("C_INBUF_ADR", INBUF_START);

    ADD_LOADER("C_HEADER_SIZE", HEADER_SIZE);
}

unsigned lineno = 0;
void repl(Ctx *c) {
    char line[1024];
    _Bool silent = 0;
    _Bool quit = 0;

    while(1) {
        silent = c->m[c->flags_ptr] >> 1 & 1;
        quit = c->m[c->flags_ptr] >> 0 & 1;
        if (quit) return;

        fgets(line, COUNTOF(line), stdin);
        lineno += 1;

        unsigned lsize = char_strlen(line)-1; // Exclude terminating newline
        if (char_strlen(line) <= 0)
            continue;

        cell_char_memcpy(&c->m[c->inbuf_start], line, lsize);
        c->m[c->inbuf_start+lsize] = '\0';
        if (!silent) printf("OUTPUT:");
        int retval = interpret(c, &c->m[c->inbuf_start], lsize, silent);
        if (retval) printf("Error line: %d\n", lineno);
    }
}

#ifndef NOMFORTH_LIB
Ctx  context;
int main(void) {
    init(&context);
    printf("Welcome to nomForth!\nTo exit, type BYE.\n");
    repl(&context);
    printf("\nThanks for using nomForth.\n");
}
#endif

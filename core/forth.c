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
void makeWord(Ctx *c, Cell *m, Cell *name, unsigned name_size,
              _Bool p, _Bool forbid_tco, _Bool allow_interpret, Cell *data, unsigned data_size) {
    Cell prev_loc = m[c->dict_pos_ptr];
    Cell prev_size = (prev_loc == 0) ? DICT_START : (3+(m[prev_loc+2] & 0x0000FFFF));
    m[c->dict_pos_ptr] = prev_loc + prev_size;

    Cell curr_loc = m[c->dict_pos_ptr];
    m[curr_loc] = prev_loc;
    m[curr_loc+1] = addToPad(c, m, name, name_size);
    m[curr_loc+2] = (forbid_tco << 30) | (allow_interpret << 29) | (p << 28) | (data_size & 0x0000FFFF);
    for (unsigned i = 0; i < data_size; i++)
        m[curr_loc+3+i] = data[i];
}
Cell appendWord(Ctx *c, Cell *m, Cell *data, Cell data_size) {
    Cell curr_loc = m[c->dict_pos_ptr];
    unsigned size = 3+(m[curr_loc+2] & 0x0000FFFF);
    for (unsigned i = 0; i < data_size; i++)
        m[curr_loc+size+i] = data[i];
    m[curr_loc+2] += data_size;

    // Pointer to the last written byte of data,
    // used for easily placing references in the data
    // stack during the compilation process
    return (curr_loc + (size-1) + data_size);
}

// Just to avoid pasting the same code twice
#define INSTANCE_FIND_WORD(__x, __y) \
__x *n = (__x *)s;\
Cell n_s = s_size;\
if (n_s == 0) return 0;\
_Bool condition = 1;\
Cell idx = m[c->dict_pos_ptr];\
while (condition) {\
    Cell temp_loc = m[idx+1];\
    Cell temp_s = m[temp_loc] - 1;\
    unsigned i;\
    _Bool are_equal = 1;\
    if (n_s != temp_s) are_equal = 0;\
    for (i = 0; i < n_s && are_equal; i++) {\
        if (toupper(m[temp_loc+1+i]) != toupper((__x)n[i])) are_equal = 0;\
    }\
    if (are_equal && (i == n_s)) condition = 0;\
    else {\
        if (m[idx] == 0) return 0;\
        else idx = m[idx];\
    }\
}\
return idx;


Cell findWord(Ctx *c, Cell *m, char strtype, void *s, unsigned s_size) {
    if (strtype == 'n') { INSTANCE_FIND_WORD(Cell, Cell); } //native string
    else if (strtype == 'c') { INSTANCE_FIND_WORD(char, unsigned char); } //character/C string
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
    m[c->program_counter_ptr] = w+3;
    _Bool silent = m[c->flags_ptr] >> 1 & 1;
    _Bool trace  = m[c->flags_ptr] >> 2 & 1;
    _Bool reached_end = 0;
    if (!silent && trace) printf("{TRACE: going to function 0x%X}\n", m[c->program_counter_ptr]);
    while (!reached_end) {
        Cell contents = m[c->program_counter_ptr];
        switch(m[contents]){
        case t_unknown_label:
            printf("{ERROR: 0 IS NOT A VALID INSTRUCTION, PC 0x%X}\n", contents);
            reached_end = 1;
            break;
        case t_nop:
            m[c->program_counter_ptr] += 1;
            break;
        case t_primitive: ;
            if (!silent && trace) printf("{TRACE: going to primitive 0x%X}\n", m[contents+1]);

            // in case the primitive tries to (or accidentally) hijack the program
            // counter, we preserve and restore it ourselves
            Cell tmp_pc = m[c->program_counter_ptr];
            executePrimitive(c, m, m[contents+1]);
            m[c->program_counter_ptr] = tmp_pc;

            m[c->program_counter_ptr] += 2;
            break;
        case t_num:
            dataPush(c, m, m[contents+1]);
            m[c->program_counter_ptr] += 2;
            break;
        case t_reljump:
            m[c->program_counter_ptr] += m[contents+1];
            break;
        case t_condreljump: {
            Cell v = dataPop(c, m);
            if (v == 0) m[c->program_counter_ptr] += m[contents+1];
            else m[c->program_counter_ptr] += 2;
            }
            break;
        case t_reljumpback:
            m[c->program_counter_ptr] -= m[contents+1];
            break;
        case t_condreljumpback: {
            Cell v = dataPop(c, m);
            if (v == 0) m[c->program_counter_ptr] -= m[contents+1];
            else m[c->program_counter_ptr] += 2;
            }
            break;
        case t_absjump:
            m[c->program_counter_ptr] = m[contents+1];
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
            if (!silent && trace) printf("{TRACE: going back to pc 0x%X from 0x%X}\n", next_pc, contents);
            if (next_pc == 0) {
                // If the next program counter is 0,
                // we're going back to the interpreter
                m[c->program_counter_ptr] = 0;
                reached_end = 1;
            } else {
                // By convention the program counter needs to be incremented from the callee
                m[c->program_counter_ptr] = next_pc;
                m[c->program_counter_ptr] += 1;
            }
            break;
        case t_execute:
            ;
            Cell new_w = 0;
            new_w = dataPop(c, m);
        default:
            if ((m[m[contents]+2] >> 30 & 1) || (m[contents+1] != t_end)) {
                funcPush(c, m, m[c->program_counter_ptr]);
            } else {
                // If it's a tail call no need to push another stack frame,
                // Unless the word is forbidden from being Tail Call Optimized
                if (!silent && trace) printf("{TRACE: Next jump is in tail position}\n");
            }
            if (m[contents] != t_execute) new_w = m[contents]; 
            contents = new_w+3;
            m[c->program_counter_ptr] = contents;
            if (!silent && trace) printf("{TRACE: jumping to function 0x%X}\n", contents);
            break;
        }
    }
}
int advanceTo(Cell **s, const Cell *max, unsigned char target, _Bool skip_leading) {
    if (skip_leading) { while (**s == target) *s += 1; }
    else { if (**s == ' ') *s += 1; }
    Cell *orig = *s;

    while (**s != target && *s <= max) *s += 1;
    Cell *now = *s;

    if (*s < max && !skip_leading) { };

    if (now > max) return -(now-orig);
    else return (now-orig);
}
void interpret(Ctx *c, Cell *m, Cell *l, unsigned l_size, _Bool silent) {
    _Bool l_c = 1;
    _Bool was_there_error = 0;
    c->inter_min = l;
    c->inter_max = l+l_size-1;
    c->inter_str = l;
    while(l_c) {
        int w_size = advanceTo(&c->inter_str, c->inter_max, ' ', 1);
        if (w_size < 0) { l_c = 0; w_size = -w_size; } else
        if (w_size == 0) break;

        Cell *lorig = c->inter_str-w_size;
        
        Cell w = findWord(c, m, 'n', lorig, w_size);
        if (w != 0) {
            /*Highest bit is for auxiliary info,
              second highest is for TCO permissions,
              third highest is 0 if it is forbidden to interpret the word,
              fourth highest is for immediacy info
            */
            _Bool priority = m[w+2]>>28 & 1;
            _Bool allow_interpreting = m[w+2]>>29 & 1;
            if (priority || m[c->compile_state_ptr] == 0) {
                if (priority && !allow_interpreting && m[c->compile_state_ptr] == 0) {
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
            *c->inter_str = '\0';
            char *endptr;

            char tmpstring[w_size+1];
            for (int i = 0; i < w_size; i++)
                tmpstring[i] = lorig[i];
            tmpstring[w_size] = '\0';
            int pos = delChar('.', tmpstring);
            _Bool valid_dot = (w_size-1);
            if (pos > 0) { //one dot
                if ((int)m[EXP] < 0)
                    valid_dot = (w_size-1)-pos == -(int)m[EXP];
                else
                    valid_dot = 0;
                w_size -= 1;
            } else if (pos == -1) // no dots, fine
                valid_dot = (int)m[EXP] >= 0;
            else //multiple dots, nonsense
                valid_dot = 0;

            Cell val = strtol(tmpstring, &endptr, m[c->base_ptr]);
            int converted_num_size = endptr-tmpstring;

            if (l_c != 0) *c->inter_str = ' ';

            if (valid_dot && converted_num_size == w_size) {
                if (m[c->compile_state_ptr] == 0)
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
void printMemory(Cell *m, unsigned start, unsigned maxval, unsigned increment) {
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
        unsigned lsize = char_strlen(line)-1; /*Exclude terminating newline*/
        if (char_strlen(line) > 1) {
            for (unsigned i = 0; i < lsize; i++)
                m[c->inbuf_start+i] = line[i];
            m[c->inbuf_start+lsize] = '\0';
            if (!silent) printf("OUTPUT:");
            interpret(c, m, &m[c->inbuf_start], lsize, silent);
        }
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

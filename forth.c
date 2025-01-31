// Copyright (c) 2025 Nomagno
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "forth.h"

Cell addToPad(Ctx *c, Cell *m, char *s, unsigned name_size) {
    if (s == NULL || *s == '\0') {
        return 0;
    }
    unsigned i;
    Cell str_loc = m[c->pad_pos_ptr];
    Cell str_size = name_size+1;
    m[str_loc]=str_size;
    for (i = 0; i < str_size-1; i++){
        m[str_loc+1+i]=s[i];
    }
    m[c->pad_pos_ptr] = str_loc + str_size;
    return str_loc;
}

void makeWord(Ctx *c, Cell *m, char *name, unsigned name_size,
              _Bool p, Cell *data, unsigned data_size) {
    Cell prev_loc = m[c->dict_pos_ptr];
    Cell prev_size = (prev_loc == 0) ? DICT_START : (3+(m[prev_loc+2] & 0xFFFF));
    m[c->dict_pos_ptr] = prev_loc + + prev_size;

    Cell curr_loc = m[c->dict_pos_ptr];
    m[curr_loc] = prev_loc;
    m[curr_loc+1] = addToPad(c, m, name, name_size);
    m[curr_loc+2] = (p << 24) | (data_size & 0x0000FFFF);
    for (unsigned i = 0; i < data_size; i++)
        m[curr_loc+3+i] = data[i];
}

Cell appendWord(Ctx *c, Cell *m, Cell *data, Cell data_size) {
    Cell curr_loc = m[c->dict_pos_ptr];
    unsigned size = 3+m[curr_loc+2];
    for (unsigned i = 0; i < data_size; i++)
        m[curr_loc+size+i] = data[i];
    m[curr_loc+2] += data_size;

    // Pointer to the last written byte of data,
    // used for easily placing references in the data
    // stack during the compilation process
    return (curr_loc + (size-1) + data_size);
}

void dataPush(Ctx *c, Cell *m, Cell v) {
    m[m[c->dstack_ptr]] = v;
    m[c->dstack_ptr] += 1;
}

Cell dataPop(Ctx *c, Cell *m) {
    if (m[c->dstack_ptr] == c->dstack_start) {
        printf("Data stack underflow. Aborting.\n");
        exit(1);
    }

    m[c->dstack_ptr] -= 1;
    Cell retval = m[m[c->dstack_ptr]];
    m[m[c->dstack_ptr]] = 0xDEADBEEF;
    return retval;
}

Cell dataPeek(Ctx *c, Cell *m) {
    return m[m[c->dstack_ptr]-1];    
}

void funcPush(Ctx *c, Cell *m, Cell v) {
    m[m[c->fstack_ptr]] = v;
    m[c->fstack_ptr] += 1;
}

Cell funcPop(Ctx *c, Cell *m) {
    if (m[c->fstack_ptr] == c->fstack_start) {
        printf("Function stack underflow. Aborting.\n");
        exit(1);
    }

    m[c->fstack_ptr] -= 1;
    Cell retval = m[m[c->fstack_ptr]];
    m[m[c->fstack_ptr]] = 0xDEADBEEF;
    return retval;
}

Cell funcPeek(Ctx *c, Cell *m) {
    return m[m[c->fstack_ptr]-1];    
}

Cell findWord(Ctx *c, Cell *m, char strtype, void *s, unsigned s_size) {
    //native name
    if (strtype == 'n') {
        Cell *n = (Cell *)s;
        Cell n_s = s_size;
        if (n_s == 0) return 0;

        _Bool condition = 1;
        Cell idx = m[c->dict_pos_ptr];
        while (condition) {
            Cell temp_loc = m[idx+1];
            Cell temp_s = m[temp_loc] - 1;

            unsigned i;
            _Bool are_equal = 1;
            if (n_s != temp_s) are_equal = 0;
            for (i = 0; i < n_s && are_equal; i++) {
                if (toupper(m[temp_loc+1+i]) != toupper(n[i])) {
                    are_equal = 0;
                }
            }
            if (are_equal && (i == n_s)) condition = 0;
            else {
                if (m[idx] == 0) return 0;
                else idx = m[idx];
            }
        }
        return idx;
    } else if (strtype == 'c') {
        char *n = (char *)s;
        Cell n_s = s_size;
        if (n_s == 0) return 0;

        _Bool condition = 1;
        Cell idx = m[c->dict_pos_ptr];
        while (condition) {
            Cell temp_loc = m[idx+1];
            Cell temp_s = m[temp_loc] - 1;

            unsigned i;
            _Bool are_equal = 1;
            if (n_s != temp_s) are_equal = 0;
            for (i = 0; i < n_s && are_equal; i++) {
                if (toupper(m[temp_loc+1+i]) != toupper((unsigned char)n[i])) {
                    are_equal = 0;
                }
            }
            if (are_equal && (i == n_s)) condition = 0;
            else {
                if (m[idx] == 0) return 0;
                else idx = m[idx];
            }
        }
        return idx;
    } else {
        return 0;
    }
}

void executePrimitive(Ctx *c, Cell *m, Cell id) {
    primTable[id].func(c, m);
}

void executeForeign(Ctx *c, Cell *m, Cell id) {
    foreignTable[id].func(c, m);
}

void executeWord(Ctx *c, Cell *m, Cell w) { 
    m[c->program_counter_ptr] = w+3;
    m[c->program_counter_ptr] += (m[w+2] & 0x00FF0000) >> 8;

    _Bool reached_end = 0;
    while (!reached_end) {
        Cell contents = m[c->program_counter_ptr];
        switch(m[contents]){
        case t_nop:
            m[c->program_counter_ptr] += 1;
            break;
        case t_primitive:
            executePrimitive(c, m, m[contents+1]);
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
            printf("Non-replaced leave labelled, this shouldn't have happened. Aborting.\n");
            reached_end = 1;
            break;         
        case t_end:
            if (m[c->fstack_ptr] - c->fstack_start == 0) {
                //If the stack size is 0,
                //we're going back to the interpreter
                //printf("Returning control to interpreter\n");
                reached_end = 1;
            } else {
                // By convention the program counter needs to be incremented from the callee
                //printf("Returning control to caller: %X, %X\n", m[c->fstack_ptr], c->fstack_start);
                m[c->program_counter_ptr] = funcPop(c, m);
                m[c->program_counter_ptr] += 1;
            }
            break;
        case t_end_notailcall:
            // Internally, the possibility of tail calls generated by an end label
            // Breaks DOES> code generation catastrophically.
            if (m[c->fstack_ptr] - c->fstack_start == 0) {
                //If the stack size is 0,
                //we're going back to the interpreter
                //printf("Returning control to interpreter\n");
                reached_end = 1;
            } else {
                // By convention the program counter needs to be incremented from the callee
                //printf("Returning control to caller: %X, %X\n", m[c->fstack_ptr], c->fstack_start);
                m[c->program_counter_ptr] = funcPop(c, m);
                m[c->program_counter_ptr] += 1;
            }
            break;
        default:
            if (m[contents+1] != t_end) { // If it's a tail call no need to push another stack frame
                funcPush(c, m, m[c->program_counter_ptr]);
            } else {
                //printf("Doing tail recursion\n");
            }
            Cell new_w = m[contents]; 
            contents = new_w+3;
            m[c->program_counter_ptr] = contents;
            m[c->program_counter_ptr] += (m[new_w+2] & 0x00FF0000) >> 8;
            break;
        }
    }
}

int advanceTo(char **s, char target, _Bool skip_leading) {
 if (skip_leading) while (**s == target) *s += 1;
 else {
  if (**s == ' ') *s += 1;
 }
    char *orig = *s;

    while (**s != target && **s != '\0') *s += 1;
    char *now = *s;

    if (**s != '\0' && !skip_leading) { };

    if (*now == '\0') return -(now-orig);
    else return (now-orig);
}

void interpret(Ctx *c, Cell *m, char *l) {
    _Bool l_c = 1;
    _Bool was_there_error = 0;
    c->inter_min = l;
    c->inter_max = l+strlen(l)-1;
    c->inter_str = l;
    while(l_c) {
        int w_size = advanceTo(&c->inter_str, ' ', 1);
        if (w_size < 0) { l_c = 0; w_size = -w_size; }
        if (w_size == 0) break;

        char *lorig = c->inter_str-w_size;
        
        Cell w = findWord(c, m, 'c', lorig, w_size);
        if (w != 0) {
            _Bool priority = m[w+2]>>24;
            if (priority || m[c->compile_state_ptr] == 0) {
                executeWord(c, m, w);
            } else {
                dataPush(c, m, w);
                PRIM(compile)(c, m);
            }
        } else {
            *c->inter_str = '\0';
            char *endptr;
            Cell val = strtol(lorig, &endptr, 10);
            if (l_c != 0) *c->inter_str = ' ';

            if (endptr-lorig == w_size) {
                if (m[w+2]>>24 != 0 || m[c->compile_state_ptr] == 0)
                    dataPush(c, m, val);
                else {
                    dataPush(c, m, val);
                    PRIM(literal)(c, m);
                }
            } else {
                printf("Error: unknown word %.*s. Aborting.\n", w_size, lorig);
                l_c = 0;
                was_there_error = 1;
            }
        }
    }
    if (!was_there_error) printf(" OK\n");
}

void init(Ctx *c, Cell *m) {
    c->compile_state_ptr = CS;
    c->program_counter_ptr = PC;
    c->dstack_start = DSTACK_START;
    c->fstack_start = FSTACK_START;
    c->dstack_ptr = DSTACK_START-1;
    c->fstack_ptr = FSTACK_START-1;
    c->dict_pos_ptr = DICT_START-1;
    c->pad_pos_ptr = PAD_START-1;

    m[c->compile_state_ptr] = 0;
    m[c->program_counter_ptr] = 0;
    m[c->dstack_ptr] = DSTACK_START;
    m[c->fstack_ptr] = FSTACK_START;
    m[c->dict_pos_ptr] = NILPTR;
    m[c->pad_pos_ptr] = PAD_START;

    initPrimitives(c, m);
}

void printMemory(Cell *m, unsigned start, unsigned maxval, unsigned increment) {
    for(unsigned i = start; i < maxval; i += increment) {
        printf("0x%4X:", i);
        for (unsigned j = i; j < i+increment; j++) {
            if (m[j] != 0) {
                printf(" \033[30;41m%08X\033[0m", m[j]);
            } else {
                 printf(" %08X", m[j]);
            }
        }
        printf("\n");
    }
}


void initPrimitives(Ctx *c, Cell *m) {
    for (unsigned i = 0; i < PRIM_NUM; i++)
    {
        if (primTable[i].func == NULL) { }
        else {
            makeWord(c, m, primTable[i].name, strlen(primTable[i].name), primTable[i].priority, CA(WT(t_primitive, i), WT(t_end)), 3);
        }
    }
}

PrimitiveData foreignTable[FOREIGN_NUM] = { };

Cell memory[MEM_MAX];
int main(void) {
    Ctx myContext;
    init(&myContext, memory);

    char line[1024];
    char *linep = &line[0];
    size_t lines = 1024;

    printMemory(memory, 0, 0x1000, 0x10);
    while(1) {
        printf("nomForth>");
        int nread = getline(&linep, &lines, stdin);
        line[nread-1] = '\0';
        if (nread == 0) line[0] = '\0';
        if (strcmp(line, "quit") == 0) {
            printf("\nThanks for using nomForth.\n");
            break;
        }
        interpret(&myContext, memory, line);
    }
}

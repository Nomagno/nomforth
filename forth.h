// Copyright (c) 2025 Nomagno
// MIT License

#ifndef _FORTH_H
#define _FORTH_H
#include <stdint.h>

typedef uint32_t Cell;

//8 bits
typedef enum {
    t_normal=0,
    t_immediate=1
} PrefType;
//Jump: a static relative jump at beggining of the word, for supported word types

//32 bits
typedef enum {
 t_unknown_label=0,
    t_nop=1, //The second cell can be used to store info that will be ignored at runtime
    t_primitive=2, //execute
    t_num=3, //push payload on the stack
    t_reljump=4, //pc+=payload
    t_reljumpback=5, //pc-=payload
    t_condreljump=6, //consume one flag.
             //   if it's true, pc+=1
             //   if it's false, pc+=payload
    t_condreljumpback=7, //consume one flag.
             //   if it's true, pc+=1
             //   if it's false, pc-=payload
    t_absjump=8, //pc=payload
    t_leavelabel=9, //to be replayed by a reljump,
                    //the payload can contain some information
                    //pertaining to the loop, like a link to the previous leave
                    //for faster traversal by the loop that must replace it
    t_end=10, // Return from word
    t_end_notailcall=11 // Return from word but ensuring this label causes no tail recursion
} CodeType;

// WORD = (Prev Name (Preference x DataSize) [Data])
// Prev: pointer to previous entry in dict
// Name: pointer to string in string list

// The dictionary pointer points to the start of the last-defined word,
// The pad, function stack, and data stack pointers point to the
//     firt nonused byte.

typedef struct {
    Cell dstack_ptr;
    Cell dstack_start;
    Cell fstack_ptr;
    Cell fstack_start;
    Cell pad_pos_ptr;
    Cell dict_pos_ptr;

    char *inter_max;
    char *inter_min;
    char *inter_str;

    Cell compile_state_ptr;
    Cell program_counter_ptr;
} Ctx;

#define FOREIGN_NUM 64
#define PRIM_NUM 256
typedef void(*forthFunc)(Ctx*,Cell*);
typedef struct {
    forthFunc func;
    _Bool priority;
    char *name;
} PrimitiveData;

extern PrimitiveData primTable[PRIM_NUM];
extern PrimitiveData foreignTable[FOREIGN_NUM];

#define MUNIT 1024
#define MEM_MAX 64*MUNIT

#define NILPTR 0x0000
#define PC 0x0001
#define CS 0x0002
#define STRPOS 0x0003
#define STRSIZE 0x0004
#define STRPTR 0x0005

#define DICT_START 0x0020
#define DSTACK_START 0x0A00
#define FSTACK_START 0x0B00
#define PAD_START    0x0C00
#define HEAP_START   0x1000

#define CA(...) (Cell[]){__VA_ARGS__}

Cell addToPad(Ctx *c, Cell *m, char *s, unsigned name_size);
void makeWord(Ctx *c, Cell *m, char *name, unsigned name_size, _Bool p,
              Cell *data, unsigned data_size);
Cell appendWord(Ctx *c, Cell *m, Cell *data, Cell data_size);
Cell findWord(Ctx *c, Cell *m, char strtype, void *s, unsigned s_size);

void executePrimitive(Ctx *c, Cell *m, Cell id);
void executeForeign(Ctx *c, Cell *m, Cell id);
void executeComposite(Ctx *c, Cell *m, Cell w);
void executeWord(Ctx *c, Cell *m, Cell w);
void interpret(Ctx *c, Cell *m, char *l);

void dataPush(Ctx *c, Cell *m, Cell v);
Cell dataPop(Ctx *c, Cell *m);
Cell dataPeek(Ctx *c, Cell *m);
void funcPush(Ctx *c, Cell *m, Cell v);
Cell funcPop(Ctx *c, Cell *m);
Cell funcPeek(Ctx *c, Cell *m);

void init(Ctx *c, Cell *m);
void initPrimitives(Ctx *c, Cell *m);
void printMemory(Cell *m, unsigned start, unsigned maxval, unsigned increment);
int advanceTo(char **s, char target, _Bool skip_leading);

#define MAKEPRIM(x) void prim_##x(Ctx *c, Cell *m)
#define PRIM(x) prim_##x

#define BOOL(x) (x ? ((Cell)-1) : 0)

#include "prims.h"

#endif

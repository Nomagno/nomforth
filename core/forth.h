// Copyright (c) 2025 Nomagno
// MIT License

#ifndef _FORTH_H
#define _FORTH_H
#include <stdint.h>

typedef uint32_t Cell;

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
    t_end_notailcall=11, // Return from word but ensuring this label causes no tail recursion
    t_execute=12 // Execute word from the stack
} CodeType;

// Execution Token = Name Token = (Prev Name Header [Data])
// Prev: pointer to previous entry in the dictionary, can be 0 for the first entry
// Name: pointer to counter string representing the word name, can be 0 for anonymous functions
// Header:
//  vtip cccc pppp pppp ssss ssss ssss ssss
//  v: If 1, this word is a "regular" variable
//  t: If 1, this word can not be tail-call optimized
//  i: If 1, this word can not be executed in interpreting mode
//  c: These 4 bits are for custom user-defined data
//  p: These 8 bits are for the word-plane. More specifically,
//       the higher 128 planes (leftmost bit set to 1) are reserved
//       for "shadowed copies" of words in the first 8 planes,
//       yielding up to 16 copies per word. The lower 128 planes
//       are for implementing alternate namespaces that must be specified
//       before their contents become findable
// s: these 16 bits represent the size of the code, in 32-bit cells, that follows the word header
//    Note this code can also be used to store data if an early unconditional early return is placed
//   (this is how "regular" variables work, they contain two bytes to push the data address onto the stack,
//    two "exit" bytes that can later be replaced by DOES> with a jump to special code, and the rest of the space is for
//    data)

// The dictionary pointer points to the start of the last-defined word,
// The pad, function stack, and data stack pointers point to the
//     firt nonused byte.

typedef struct {
    Cell dstack_ptr;
    Cell fstack_ptr;
    Cell flags_ptr;
    Cell pad_pos_ptr;
    Cell dict_pos_ptr;
    Cell heap_start;
    Cell inbuf_start;

    Cell *input_end;
    Cell *input_start;
    Cell *input;

    Cell base_ptr;
    Cell exp_ptr;
    Cell compile_state_ptr;
    Cell program_counter_ptr;

    Cell *m;
} Ctx;

#define PRIM_NUM 256
typedef void(*forthFunc)(Ctx*);
typedef struct {
    forthFunc func;
    _Bool priority;
    _Bool forbid_tco;
    _Bool allow_interpret;
    char *name;
} PrimitiveData;

extern PrimitiveData primTable[PRIM_NUM];

#define MEM_START(x) (Cell)(x+1)

#define MUNIT 1024
#define MEM_MAX 128*MUNIT

#define NILPTR 0x0000
#define PC 0x0001
#define CS 0x0002
#define STRPOS 0x0003
#define STRSIZE 0x0004
#define STRPTR 0x0005
#define FLAGS 0x0006
#define BASE  0x0007
#define EXP  0x0008

#define DICT_START    0x0020
#define DSTACK_START  0x14000
#define FSTACK_START  0x15000
#define USERMEM_START 0x16000
#define HEAP_START    0x18000
#define PAD_START     0x1A000
#define INBUF_START   0x1F000

#define CA(...) (Cell[]){__VA_ARGS__}

void dataPush(Ctx *c, Cell v);
Cell dataPop(Ctx *c);
Cell dataPeek(Ctx *c);
void funcPush(Ctx *c, Cell v);
Cell funcPop(Ctx *c);
Cell funcPeek(Ctx *c);

Cell addToPad(Ctx *c, Cell *s, unsigned name_size);
void makeWord(Ctx *c, Cell *name, unsigned name_size, _Bool p, _Bool forbid_tco,
              _Bool forbid_interpreting, Cell *data, unsigned data_size);
Cell appendWord(Ctx *c, Cell *data, Cell data_size);
Cell findWord(Ctx *c, char strtype, void *s, unsigned s_size);

void executePrimitive(Ctx *c, Cell id);
void executeWord(Ctx *c, Cell w);
int consumeWord(Cell **s, const Cell *max, unsigned char target, _Bool skip_leading);
int interpret(Ctx *c, Cell *l, unsigned l_size, _Bool silent);

void init(Ctx *c);
void initPrimitives(Ctx *c);

void repl(Ctx *c);

#define MAKEPRIM(x) void prim_##x(Ctx *c)
#define PRIM(x) prim_##x

#define BOOL(x) (x ? ((Cell)-1) : 0)

#include "../primitives/prims.h"



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
#define EXTRACT_SIZE(__x) (3+(c->m[__x+2] & 0x0000FFFF))
#define GET_PREV(__x, ...) (c->m[__x] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_NAME(__x, ...) (c->m[__x+1] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_HEADER(__x, ...) (c->m[__x+2] __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN))
#define GET_DATA(__x, ...) (c->m[__x + 3 __VA_OPT__(+) __VA_OPT__(LPAREN) __VA_ARGS__ __VA_OPT__(RPAREN)])
#define COMPILE_STATE (c->m[c->compile_state_ptr])
#define DICTPTR (c->m[c->dict_pos_ptr])
#define PROGRAM_COUNTER (c->m[c->program_counter_ptr])
#define EXP_PTR c->m[c->exp_ptr]
#define BASE_PTR c->m[c->base_ptr]

#endif

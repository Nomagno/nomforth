#ifndef _OA_H
#define _OA_H
#include <stdint.h>
/* Copyright 2025 nomagno*/
/* CC0 License */

/*Offset-based allocator (can be used with custom heaps)*/

/*Replace 32 for however many bits you want.*/
/*For N bits the maximum heap size is (2^N)-2*/
typedef uint32_t wptr;
#define NIL UINT32_MAX
#define USED_MARK UINT32_MAX-1

extern wptr *H; /* BEHOLD: THE HEAP! */
extern wptr H_L; /* Last offset of the heap */
extern const wptr F; /* linked list of free nodes, lives within the heap*/

/*Public interface:*/
void OA_init(wptr *base, unsigned s);
wptr OA_malloc(unsigned s);
void OA_free(wptr n);
void OA_defrag();

/*Internal functions:*/
wptr OA_split(wptr n, unsigned s);
wptr OA_raw_alloc(unsigned s);
#endif

#include "oa.h"

/* Copyright 2025 nomagno*/
/* CC0 License */

/*Offset-based allocator (can be used with custom heaps)*/

#define SIZE(x) H[x]
#define NEXT(x) H[x+1]
#define USED(x) (NEXT(x) == USED_MARK)

wptr *H;
wptr H_L;
const wptr F = 0;

void OA_init(wptr *base, unsigned s) {
  H = base;
  /*F = 0;*/
  H_L = s-1;
  *H = s-2; /* size */
  *(H+1) = NIL; /* pointer to next */
}

wptr OA_split(wptr n, unsigned s) {
    wptr tail = (n+SIZE(n)+2)-(s+2);
    SIZE(tail) = s;
    NEXT(tail) = USED_MARK;
    SIZE(n) = SIZE(n)-(s+2);
    return tail;
}

wptr OA_raw_alloc(unsigned s) {
  wptr c = F, b = NIL;
  /* Find the smallest node that can still fit ``s""+2 cells
   *  (+2 because we need to split it and the remainder
   *  must be able to fit at least a header*/
  while (c != NIL) {
    if (SIZE(c) >= (s+2) && (b == NIL || SIZE(c) < SIZE(b)))
        b = c;
    c = NEXT(c); /*Go to next node*/
  }
  if (b == NIL) return NIL;
  return OA_split(b, s);
}

void OA_defrag() {
    wptr c = 0, p;
    /*If the first node is also the last node,
      there is nothing to merge*/
    if (NEXT(c) == NIL) return;
    p = c;
    c += SIZE(c)+2;
    while(c <= H_L) {
        if (!USED(p) && !USED(c)) {
            SIZE(p) += SIZE(c)+2;
            NEXT(p) = NEXT(c);
        }
        p = c;
        c += SIZE(c)+2;
    }
}

/*Lazy defrag strat, only try if allocation failed*/
wptr OA_malloc(unsigned s) {
    wptr retval = OA_raw_alloc(s);
    if (retval == NIL) {
        OA_defrag();
        retval = OA_raw_alloc(s);
    }
    if (retval != NIL) return retval+2;
    else return NIL;
}

void OA_free(wptr n) {
    wptr c = F, tmp;
    if (n == NIL) return;
    n -= 2;
    while (NEXT(c) != NIL && NEXT(c) < n) c = NEXT(c);
    tmp = NEXT(c);
    NEXT(c) = n;
    NEXT(n) = tmp;
}

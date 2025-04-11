// Copyright (c) 2025 Nomagno
// MIT License

#ifndef _PRIMS_H
#define _PRIMS_H
#include "../core/forth.h"
#include "oa.h"

MAKEPRIM(colon);
MAKEPRIM(colonAnon);
MAKEPRIM(semicolon);
MAKEPRIM(leftparen);
MAKEPRIM(backslash);
MAKEPRIM(bye);

MAKEPRIM(emptyword);
MAKEPRIM(create);
MAKEPRIM(comma);
MAKEPRIM(worddoesprim);
MAKEPRIM(worddoes);

MAKEPRIM(fetch);
MAKEPRIM(store);
MAKEPRIM(rget);
MAKEPRIM(rsend);

MAKEPRIM(b_char_b);
MAKEPRIM(char);
MAKEPRIM(word);
MAKEPRIM(parse);
MAKEPRIM(parse_name);
MAKEPRIM(find);
MAKEPRIM(is);
MAKEPRIM(defer);
MAKEPRIM(postpone);
MAKEPRIM(getchar);
MAKEPRIM(getnum);
MAKEPRIM(accept);
MAKEPRIM(flushoutput);
MAKEPRIM(evaluate);

MAKEPRIM(heap_init);
MAKEPRIM(defrag);
MAKEPRIM(allocate);
MAKEPRIM(free);

MAKEPRIM(add);
MAKEPRIM(minus);
MAKEPRIM(mult);
MAKEPRIM(div);
MAKEPRIM(div);
MAKEPRIM(mod);
MAKEPRIM(rshift);
MAKEPRIM(lshift);
MAKEPRIM(max);
MAKEPRIM(min);
MAKEPRIM(arith_not);
MAKEPRIM(bitwise_not);
MAKEPRIM(logical_not);
MAKEPRIM(and);
MAKEPRIM(or);
MAKEPRIM(xor);
MAKEPRIM(abs);
MAKEPRIM(eq);
MAKEPRIM(neq);
MAKEPRIM(le);
MAKEPRIM(leq);
MAKEPRIM(gr);
MAKEPRIM(geq);

MAKEPRIM(emit);
MAKEPRIM(type);
MAKEPRIM(cr);
MAKEPRIM(spaces);
MAKEPRIM(dot);
MAKEPRIM(udot);
MAKEPRIM(xdot);
MAKEPRIM(dotmem);
MAKEPRIM(dotstack);
MAKEPRIM(udotstack);
MAKEPRIM(dotstackret);
MAKEPRIM(udotstackret);

MAKEPRIM(drop);
MAKEPRIM(nip);
MAKEPRIM(dup);
MAKEPRIM(over);
MAKEPRIM(swap);
MAKEPRIM(rot);
MAKEPRIM(tuck);

MAKEPRIM(2fetch);
MAKEPRIM(2store);
MAKEPRIM(2rget);
MAKEPRIM(2rsend);
MAKEPRIM(2drop);
MAKEPRIM(2nip);
MAKEPRIM(2dup);
MAKEPRIM(2over);
MAKEPRIM(2swap);
MAKEPRIM(2rot);
MAKEPRIM(2tuck);

MAKEPRIM(rdrop);
MAKEPRIM(rnip);
MAKEPRIM(rdup);
MAKEPRIM(rover);
MAKEPRIM(rswap);
MAKEPRIM(rrot);
MAKEPRIM(rtuck);

#define     IMMEDIATE_WORD 1
#define        NORMAL_WORD 0

#define    ALLOW_INTERPRET 1
#define DISALLOW_INTERPRET 0
#define PERM_DOESNOT_APPLY 0

#define         FORBID_TCO 1
#define          ALLOW_TCO 0

#define PRIM_TABLE_DEFAULT {                                                           \
    {PRIM(colon),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ":"},         \
    {PRIM(colonAnon),       NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ":NONAME"},   \
    {PRIM(semicolon),    IMMEDIATE_WORD,  ALLOW_TCO, DISALLOW_INTERPRET, ";"},         \
                                                                                       \
    {PRIM(leftparen),    IMMEDIATE_WORD,  ALLOW_TCO,    ALLOW_INTERPRET, "("},         \
    {PRIM(backslash),    IMMEDIATE_WORD,  ALLOW_TCO,    ALLOW_INTERPRET, "\\"},        \
                                                                                       \
    {PRIM(bye),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "BYE"},       \
                                                                                       \
    {PRIM(emptyword),       NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "EMPTY_WORD"},\
    {PRIM(create),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "CREATE"},    \
    {PRIM(comma),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ","},         \
    {PRIM(worddoesprim),    NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DOES>PRIM"}, \
    {PRIM(worddoes),     IMMEDIATE_WORD,  ALLOW_TCO, DISALLOW_INTERPRET, "DOES>"},     \
                                                                                       \
    {PRIM(fetch),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "@"},         \
    {PRIM(store),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "!"},         \
    {PRIM(rget),            NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, ">R"},        \
    {PRIM(rsend),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "R>"},        \
                                                                                       \
    {PRIM(b_char_b),     IMMEDIATE_WORD,  ALLOW_TCO, DISALLOW_INTERPRET, "[CHAR]"},    \
    {PRIM(char),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "CHAR"},      \
    {PRIM(word),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "WORD"},      \
    {PRIM(parse),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "PARSE"},     \
    {PRIM(parse_name),      NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "PARSE-NAME"},\
    {PRIM(find),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "FIND"},      \
    {PRIM(is),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "IS"},        \
    {PRIM(defer),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DEFER"},     \
    {PRIM(postpone),     IMMEDIATE_WORD,  ALLOW_TCO, DISALLOW_INTERPRET, "POSTPONE"},  \
    {PRIM(getchar),         NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "GETC"},      \
    {PRIM(getnum),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "GETN"},      \
    {PRIM(accept),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "ACCEPT"},      \
    {PRIM(flushoutput),     NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "FLUSH_OUT"}, \
    {PRIM(evaluate),        NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "EVALUATE"},  \
    {PRIM(defer),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DEFER"},     \
                                                                                       \
    {PRIM(heap_init),       NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "HEAP_INIT"}, \
    {PRIM(defrag),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DEFRAG"},    \
    {PRIM(allocate),        NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "ALLOCATE"},  \
    {PRIM(free),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "free"},      \
                                                                                       \
    {PRIM(add),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "+"},         \
    {PRIM(minus),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "-"},         \
    {PRIM(mult),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "*"},         \
    {PRIM(div),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "/"},         \
    {PRIM(mod),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "MOD"},       \
    {PRIM(rshift),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "RSHIFT"},    \
    {PRIM(lshift),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "LSHIFT"},    \
    {PRIM(max),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "MAX"},       \
    {PRIM(min),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "MIN"},       \
    {PRIM(arith_not),       NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "ARITHMETICAL_NOT"},\
    {PRIM(bitwise_not),     NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "BITWISE_NOT"},\
    {PRIM(logical_not),     NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "LOGICAL_NOT"},\
    {PRIM(and),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "AND"},       \
    {PRIM(or),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "OR"},        \
    {PRIM(xor),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "XOR"},       \
    {PRIM(abs),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "ABS"},       \
    {PRIM(eq),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "="},         \
    {PRIM(neq),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "!="},        \
    {PRIM(le),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "<"},         \
    {PRIM(leq),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "<="},        \
    {PRIM(gr),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ">"},         \
    {PRIM(geq),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ">="},        \
                                                                                       \
    {PRIM(emit),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "EMIT"},      \
    {PRIM(type),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "TYPE"},      \
    {PRIM(cr),              NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "CR"},        \
    {PRIM(spaces),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "SPACES"},    \
    {PRIM(dot),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "."},         \
    {PRIM(udot),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "U."},        \
    {PRIM(xdot),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "X."},        \
    {PRIM(dotmem),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "M."},        \
    {PRIM(dotstack),        NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, ".S"},        \
    {PRIM(udotstack),       NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "U.S"},       \
    {PRIM(dotstackret),     NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "R.S"},       \
    {PRIM(udotstackret),    NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "UR.S"},      \
                                                                                       \
    {PRIM(drop),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DROP"},      \
    {PRIM(nip),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "NIP"},       \
    {PRIM(dup),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "DUP"},       \
    {PRIM(over),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "OVER"},      \
    {PRIM(swap),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "SWAP"},      \
    {PRIM(rot),             NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "ROT"},       \
    {PRIM(tuck),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "TUCK"},      \
                                                                                       \
    {PRIM(2fetch),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2@"},        \
    {PRIM(2store),          NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2!"},        \
    {PRIM(2rget),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "2>R"},       \
    {PRIM(2rsend),          NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "2R>"},       \
    {PRIM(2drop),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2DROP"},     \
    {PRIM(2nip),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2NIP"},      \
    {PRIM(2dup),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2DUP"},      \
    {PRIM(2over),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2OVER"},     \
    {PRIM(2swap),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2SWAP"},     \
    {PRIM(2rot),            NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2ROT"},      \
    {PRIM(2tuck),           NORMAL_WORD,  ALLOW_TCO, PERM_DOESNOT_APPLY, "2TUCK"},     \
                                                                                       \
    {PRIM(rdrop),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RDROP"},     \
    {PRIM(rnip),            NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RNIP"},      \
    {PRIM(rdup),            NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RDUP"},      \
    {PRIM(rover),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "ROVER"},     \
    {PRIM(rswap),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RSWAP"},     \
    {PRIM(rrot),            NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RROT"},      \
    {PRIM(rtuck),           NORMAL_WORD, FORBID_TCO, PERM_DOESNOT_APPLY, "RTUCK"},     \
}

#endif

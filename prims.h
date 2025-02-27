// Copyright (c) 2025 Nomagno
// MIT License

#ifndef _PRIMS_H
#define _PRIMS_H
#include "forth.h"

MAKEPRIM(colon);
MAKEPRIM(colonAnonymous);
MAKEPRIM(semicolon);
MAKEPRIM(leftparen);
MAKEPRIM(backslash);

MAKEPRIM(emptyword);
MAKEPRIM(create);
MAKEPRIM(comma);
MAKEPRIM(worddoesprim);
MAKEPRIM(worddoes);

MAKEPRIM(fetch);
MAKEPRIM(store);
MAKEPRIM(rget);
MAKEPRIM(rsend);

MAKEPRIM(bracket_char_bracket);
MAKEPRIM(char);
MAKEPRIM(word);
MAKEPRIM(parse);
MAKEPRIM(parse_name);
MAKEPRIM(find);
MAKEPRIM(is);
MAKEPRIM(defer);
MAKEPRIM(postpone);
MAKEPRIM(getchar);


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
MAKEPRIM(arithmetical_not);
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
MAKEPRIM(dotstackreturn);
MAKEPRIM(udotstackreturn);

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

#define PRIM_TABLE_DEFAULT { \
    {PRIM(colon), 0, 0, 0, ":"}, \
    {PRIM(colonAnonymous), 0, 0, 0, ":NONAME"}, \
    {PRIM(semicolon), 1, 0, 0, ";"}, \
                            \
    {PRIM(leftparen), 1, 0, 0, "("}, \
    {PRIM(backslash), 1, 0, 0, "\\"}, \
                            \
    {PRIM(emptyword), 0, 0, 0, "EMPTY_WORD"}, \
    {PRIM(create), 0, 0, 0, "CREATE"}, \
    {PRIM(comma), 0, 0, 0, ","}, \
    {PRIM(worddoesprim), 0, 0, 0, "DOES>PRIM"}, \
    {PRIM(worddoes), 1, 0, 0, "DOES>"}, \
                            \
    {PRIM(fetch), 0, 0, 0, "@"}, \
    {PRIM(store), 0, 0, 0, "!"}, \
    {PRIM(rget), 0, 1, 0, ">R"}, \
    {PRIM(rsend), 0, 1, 0, "R>"}, \
                            \
    {PRIM(bracket_char_bracket), 1, 0, 0, "[CHAR]"}, \
    {PRIM(char), 1, 0, 0, "CHAR"}, \
    {PRIM(word), 0, 0, 0, "WORD"}, \
    {PRIM(parse), 0, 0, 0, "PARSE"}, \
    {PRIM(parse_name), 0, 0, 0, "PARSE-NAME"}, \
    {PRIM(find), 0, 0, 0, "FIND"}, \
    {PRIM(is), 0, 0, 0, "IS"}, \
    {PRIM(defer), 0, 0, 0, "DEFER"}, \
    {PRIM(postpone), 1, 0, 0, "POSTPONE"}, \
    {PRIM(getchar), 0, 0, 0, "GETC"}, \
                            \
    {PRIM(add), 0, 0, 0, "+"}, \
    {PRIM(minus), 0, 0, 0, "-"}, \
    {PRIM(mult), 0, 0, 0, "*"}, \
    {PRIM(div), 0, 0, 0, "/"}, \
    {PRIM(mod), 0, 0, 0, "MOD"}, \
    {PRIM(rshift), 0, 0, 0, "RSHIFT"}, \
    {PRIM(lshift), 0, 0, 0, "LSHIFT"}, \
    {PRIM(max), 0, 0, 0, "MAX"}, \
    {PRIM(min), 0, 0, 0, "MIN"}, \
    {PRIM(arithmetical_not), 0, 0, 0, "ARITHMETICAL_NOT"}, \
    {PRIM(bitwise_not), 0, 0, 0, "BITWISE_NOT"}, \
    {PRIM(logical_not), 0, 0, 0, "LOGICAL_NOT"}, \
    {PRIM(and), 0, 0, 0, "AND"}, \
    {PRIM(or), 0, 0, 0, "OR"}, \
    {PRIM(xor), 0, 0, 0, "XOR"}, \
    {PRIM(abs), 0, 0, 0, "ABS"}, \
    {PRIM(eq), 0, 0, 0, "="}, \
    {PRIM(neq), 0, 0, 0, "!="}, \
    {PRIM(le), 0, 0, 0, "<"}, \
    {PRIM(leq), 0, 0, 0, "<="}, \
    {PRIM(gr), 0, 0, 0, ">"}, \
    {PRIM(geq), 0, 0, 0, ">="}, \
                            \
    {PRIM(emit), 0, 0, 0, "EMIT"}, \
    {PRIM(type), 0, 0, 0, "TYPE"}, \
    {PRIM(cr), 0, 0, 0, "CR"}, \
    {PRIM(spaces), 0, 0, 0, "SPACES"}, \
    {PRIM(dot), 0, 0, 0, "."}, \
    {PRIM(udot), 0, 0, 0, "U."}, \
    {PRIM(xdot), 0, 0, 0, "X."}, \
    {PRIM(dotmem), 0, 0, 0, "M."}, \
    {PRIM(dotstack), 0, 0, 0, ".S"}, \
    {PRIM(udotstack), 0, 0, 0, "U.S"}, \
    {PRIM(dotstackreturn), 0, 0, 0, "R.S"}, \
    {PRIM(udotstackreturn), 0, 0, 0, "UR.S"}, \
                            \
    {PRIM(drop), 0, 0, 0, "DROP"}, \
    {PRIM(nip), 0, 0, 0, "NIP"}, \
    {PRIM(dup), 0, 0, 0, "DUP"}, \
    {PRIM(over), 0, 0, 0, "OVER"}, \
    {PRIM(swap), 0, 0, 0, "SWAP"}, \
    {PRIM(rot), 0, 0, 0, "ROT"}, \
    {PRIM(tuck), 0, 0, 0, "TUCK"}, \
                            \
    {PRIM(2fetch), 0, 0, 0, "2@"}, \
    {PRIM(2store), 0, 0, 0, "2!"}, \
    {PRIM(2rget), 0, 1, 0, "2>R"}, \
    {PRIM(2rsend), 0, 1, 0, "2R>"}, \
    {PRIM(2drop), 0, 0, 0, "2DROP"}, \
    {PRIM(2nip), 0, 0, 0, "2NIP"}, \
    {PRIM(2dup), 0, 0, 0, "2DUP"}, \
    {PRIM(2over), 0, 0, 0, "2OVER"}, \
    {PRIM(2swap), 0, 0, 0, "2SWAP"}, \
    {PRIM(2rot), 0, 0, 0, "2ROT"}, \
    {PRIM(2tuck), 0, 0, 0, "2TUCK"}, \
                            \
    {PRIM(rdrop), 0, 1, 0, "RDROP"}, \
    {PRIM(rnip), 0, 1, 0, "RNIP"}, \
    {PRIM(rdup), 0, 1, 0, "RDUP"}, \
    {PRIM(rover), 0, 1, 0, "ROVER"}, \
    {PRIM(rswap), 0, 1, 0, "RSWAP"}, \
    {PRIM(rrot), 0, 1, 0, "RROT"}, \
    {PRIM(rtuck), 0, 1, 0, "RTUCK"}, \
}

#endif

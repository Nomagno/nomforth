// Copyright (c) 2025 Nomagno
// MIT License

#ifndef _PRIMS_H
#define _PRIMS_H
#include "forth.h"

MAKEPRIM(colon);
MAKEPRIM(colonAnonymous);
MAKEPRIM(semicolon);
MAKEPRIM(immediate);
MAKEPRIM(forbid_tco);
MAKEPRIM(leftbracket);
MAKEPRIM(rightbracket);
MAKEPRIM(recurse);
MAKEPRIM(leftparen);
MAKEPRIM(backslash);

MAKEPRIM(exit);
MAKEPRIM(if);
MAKEPRIM(ahead);
MAKEPRIM(then);
MAKEPRIM(begin);
MAKEPRIM(again);
MAKEPRIM(until);

MAKEPRIM(variablerawsize);
MAKEPRIM(emptyvariable);
MAKEPRIM(variable);
MAKEPRIM(literal);
MAKEPRIM(comma);
MAKEPRIM(allot);
MAKEPRIM(here);
MAKEPRIM(getwordbodysize);
MAKEPRIM(getwordbody);
MAKEPRIM(worddoesprim);
MAKEPRIM(worddoes);
MAKEPRIM(move);

MAKEPRIM(bracket_char_bracket);
MAKEPRIM(char);
MAKEPRIM(word);
MAKEPRIM(parse);
MAKEPRIM(parse_name);
MAKEPRIM(find);
MAKEPRIM(who);
MAKEPRIM(is);
MAKEPRIM(defer);
MAKEPRIM(postpone);
MAKEPRIM(execute);

MAKEPRIM(add);
MAKEPRIM(minus);
MAKEPRIM(mult);
MAKEPRIM(div);
MAKEPRIM(div);
MAKEPRIM(mod);
MAKEPRIM(invert);
MAKEPRIM(rshift);
MAKEPRIM(lshift);
MAKEPRIM(max);
MAKEPRIM(min);
MAKEPRIM(negate);
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

MAKEPRIM(fetch);
MAKEPRIM(store);
MAKEPRIM(rget);
MAKEPRIM(rsend);

MAKEPRIM(2fetch);
MAKEPRIM(2store);
MAKEPRIM(2rget);
MAKEPRIM(2rsend);

MAKEPRIM(count);
MAKEPRIM(emit);
MAKEPRIM(type);
MAKEPRIM(cr);
MAKEPRIM(spaces);
MAKEPRIM(bl);
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
    {PRIM(colon), 0, 0, ":"}, \
    {PRIM(colonAnonymous), 0, 0, ":NONAME"}, \
    {PRIM(semicolon), 1, 0, ";"}, \
    {PRIM(immediate), 0, 0, "IMMEDIATE"}, \
    {PRIM(forbid_tco), 0, 0, "FORBID_TCO"}, \
    {PRIM(leftbracket), 1, 0, "["}, \
    {PRIM(rightbracket), 0, 0, "]"}, \
    {PRIM(recurse), 1, 0, "RECURSE"}, \
    {PRIM(leftparen), 1, 0, "("}, \
    {PRIM(backslash), 1, 0, "\\"}, \
                            \
    {PRIM(exit), 1, 0, "EXIT"}, \
    {PRIM(if), 1, 0, "IF"}, \
    {PRIM(ahead), 1, 0, "AHEAD"}, \
    {PRIM(then), 1, 0, "THEN"}, \
    {PRIM(begin), 1, 0, "BEGIN"}, \
    {PRIM(again), 1, 0, "AGAIN"}, \
    {PRIM(until), 1, 0, "UNTIL"}, \
                            \
    {PRIM(variablerawsize), 0, 0, "RAW_VAR_SIZE"}, \
    {PRIM(emptyvariable), 0, 0, "EMPTY_VAR"}, \
    {PRIM(variable), 0, 0, "VARIABLE"}, \
    {PRIM(literal), 0, 0, "LIT,"}, \
    {PRIM(comma), 0, 0, ","}, \
    {PRIM(allot), 0, 0, "ALLOT"}, \
    {PRIM(here), 0, 0, "HERE"}, \
    {PRIM(getwordbodysize), 0, 0, ">SIZE"}, \
    {PRIM(getwordbody), 0, 0, ">BODY"}, \
    {PRIM(worddoesprim), 0, 0, "DOES>PRIM"}, \
    {PRIM(worddoes), 1, 0, "DOES>"}, \
    {PRIM(move), 0, 0, "MOVE"}, \
                            \
    {PRIM(bracket_char_bracket), 1, 0, "[CHAR]"}, \
    {PRIM(char), 1, 0, "CHAR"}, \
    {PRIM(word), 0, 0, "WORD"}, \
    {PRIM(parse), 0, 0, "PARSE"}, \
    {PRIM(parse_name), 0, 0, "PARSE-NAME"}, \
    {PRIM(find), 0, 0, "FIND"}, \
    {PRIM(who), 0, 0, "WHO"}, \
    {PRIM(is), 0, 0, "IS"}, \
    {PRIM(defer), 0, 0, "DEFER"}, \
    {PRIM(postpone), 1, 0, "POSTPONE"}, \
    {PRIM(execute), 0, 0, "EXECUTE"}, \
                            \
    {PRIM(add), 0, 0, "+"}, \
    {PRIM(minus), 0, 0, "-"}, \
    {PRIM(mult), 0, 0, "*"}, \
    {PRIM(div), 0, 0, "/"}, \
    {PRIM(mod), 0, 0, "MOD"}, \
    {PRIM(invert), 0, 0, "INVERT"}, \
    {PRIM(rshift), 0, 0, "RSHIFT"}, \
    {PRIM(lshift), 0, 0, "LSHIFT"}, \
    {PRIM(max), 0, 0, "MAX"}, \
    {PRIM(min), 0, 0, "MIN"}, \
    {PRIM(negate), 0, 0, "NEGATE"}, \
    {PRIM(and), 0, 0, "AND"}, \
    {PRIM(or), 0, 0, "OR"}, \
    {PRIM(xor), 0, 0, "XOR"}, \
    {PRIM(abs), 0, 0, "ABS"}, \
    {PRIM(eq), 0, 0, "="}, \
    {PRIM(neq), 0, 0, "!="}, \
    {PRIM(le), 0, 0, "<"}, \
    {PRIM(leq), 0, 0, "<="}, \
    {PRIM(gr), 0, 0, ">"}, \
    {PRIM(geq), 0, 0, ">="}, \
                            \
    {PRIM(fetch), 0, 0, "@"}, \
    {PRIM(store), 0, 0, "!"}, \
    {PRIM(rget), 0, 1, ">R"}, \
    {PRIM(rsend), 0, 1, "R>"}, \
                            \
    {PRIM(2fetch), 0, 0, "2@"}, \
    {PRIM(2store), 0, 0, "2!"}, \
    {PRIM(2rget), 0, 1, "2>R"}, \
    {PRIM(2rsend), 0, 1, "2R>"}, \
                            \
    {PRIM(count), 0, 0, "COUNT"}, \
    {PRIM(bl), 0, 0, "BL"}, \
    {PRIM(emit), 0, 0, "EMIT"}, \
    {PRIM(type), 0, 0, "TYPE"}, \
    {PRIM(cr), 0, 0, "CR"}, \
    {PRIM(spaces), 0, 0, "SPACES"}, \
    {PRIM(dot), 0, 0, "."}, \
    {PRIM(udot), 0, 0, "U."}, \
    {PRIM(xdot), 0, 0, "X."}, \
    {PRIM(dotmem), 0, 0, "M."}, \
    {PRIM(dotstack), 0, 0, ".S"}, \
    {PRIM(udotstack), 0, 0, "U.S"}, \
    {PRIM(dotstackreturn), 0, 0, "R.S"}, \
    {PRIM(udotstackreturn), 0, 0, "UR.S"}, \
                            \
    {PRIM(drop), 0, 0, "DROP"}, \
    {PRIM(nip), 0, 0, "NIP"}, \
    {PRIM(dup), 0, 0, "DUP"}, \
    {PRIM(over), 0, 0, "OVER"}, \
    {PRIM(swap), 0, 0, "SWAP"}, \
    {PRIM(rot), 0, 0, "ROT"}, \
    {PRIM(tuck), 0, 0, "TUCK"}, \
                            \
    {PRIM(2drop), 0, 0, "2DROP"}, \
    {PRIM(2nip), 0, 0, "2NIP"}, \
    {PRIM(2dup), 0, 0, "2DUP"}, \
    {PRIM(2over), 0, 0, "2OVER"}, \
    {PRIM(2swap), 0, 0, "2SWAP"}, \
    {PRIM(2rot), 0, 0, "2ROT"}, \
    {PRIM(2tuck), 0, 0, "2TUCK"}, \
                            \
    {PRIM(rdrop), 0, 1, "RDROP"}, \
    {PRIM(rnip), 0, 1, "RNIP"}, \
    {PRIM(rdup), 0, 1, "RDUP"}, \
    {PRIM(rover), 0, 1, "ROVER"}, \
    {PRIM(rswap), 0, 1, "RSWAP"}, \
    {PRIM(rrot), 0, 1, "RROT"}, \
    {PRIM(rtuck), 0, 1, "RTUCK"}, \
}

#endif

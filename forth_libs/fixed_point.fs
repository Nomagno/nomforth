VARIABLE FP_BASE
VARIABLE FP_EXP
10 FP_BASE !
0  FP_EXP !

: NORMAL 10 FP_BASE !   0 FP_EXP ! ;
: MONEY  10 FP_BASE !  -2 FP_EXP ! ;
: KILO   10 FP_BASE !   3 FP_EXP ! ;

: MUL * ;
: DIV / ;

: POW ( b e -- b^e )
    DUP 0 <
    IF STRLIT" ERR: NEG EXP" CR BYE THEN

    2DUP 0 = SWAP 0 =
    AND IF STRLIT" ERR: 0^0" CR BYE THEN

    1 SWAP
    0 ?DO ( b b^i)
      SWAP DUP ROT * ( b b^[i+1])
    LOOP
    SWAP DROP
;

: *
    MUL FP_BASE @
    FP_EXP @ DUP 0 < IF
      ARITHMETICAL_NOT POW DIV
    ELSE
      POW MUL
    THEN
;

: /
    DIV FP_BASE @
    FP_EXP @ DUP 0 < IF
      ARITHMETICAL_NOT POW MUL
    ELSE
      POW DIV
    THEN
;

: FIX> 1 1 / DIV ;
: >FIX 1 1 / MUL ;

VARIABLE FP_BASE
VARIABLE FP_EXP
: FP_INF ( -- b e ) FP_BASE @ FP_EXP @ ;

: NORMAL 10 FP_BASE !   0 FP_EXP ! ;
: MONEY  10 FP_BASE !  -2 FP_EXP ! ;
: KILO   10 FP_BASE !   3 FP_EXP ! ;
NORMAL

: MUL * ;  : DIV / ;
: POW ( b e -- b^e )
    DUP 0 < ABORT" FixP: NEG EXP"
    2DUP OR 0 = ABORT" FixP: 0^0"
    1 SWAP 0 ?DO  OVER *  LOOP NIP ;
: NEG ARITHMETICAL_NOT ;
: APPLY_POW  ( u b e -- u/b^[-e], or u*b^e )
    DUP 0 <
    IF NEG POW DIV ELSE POW MUL THEN ;

: * MUL FP_INF     APPLY_POW ;
: / DIV FP_INF NEG APPLY_POW ;

: FIX> FP_INF     APPLY_POW ;
: >FIX FP_INF NEG APPLY_POW ;

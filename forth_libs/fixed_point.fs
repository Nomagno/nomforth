DECIMAL    0 EXP !
: NEG ARITHMETICAL_NOT ;
: FP_INF ( -- b e ) BASE @ EXP @ ;
: NORMAL 10 BASE !   0 EXP ! ;
: MONEY  10 BASE !  -2 EXP ! ;
: KILO   10 BASE !   3 EXP ! ;
: POW ( b e -- b^e )
    DUP     0 < ABORT" FixP: NEG EXP"
    2DUP OR 0 = ABORT" FixP: 0^0"
    1 SWAP 0 ?DO  OVER *  LOOP NIP ;
: APPLY_POW  ( u b e -- u/b^[-e], or u*b^e )
    DUP 0 < IF   NEG POW /
            ELSE     POW * THEN ;
: *  * FP_INF     APPLY_POW ;
: /  / FP_INF NEG APPLY_POW ;
: FIX> FP_INF     APPLY_POW ;
: >FIX FP_INF NEG APPLY_POW ;

( Nomagno's Forth complex standard words )
( Copyright 2025 Nomagno )
( MIT License )

( Makes nomforth usable by adding: )
( double integer, yarnball control, string functions, abort, second-class anonymous functions, pseudo-locals...)

: tri_less ( x y z -- z < x && z < y )
    dup rot < -rot swap < and ;

: d+ swap 2swap swap 2swap rot swap 2dup + dup >R tri_less LOGICAL_NOT R> + + swap ;
: d- swap 2swap swap 2swap rot swap - dup 0 < arithmetical_not ( if 0 : -1, if -1: 0 ) swap >R + - R> swap ;
: d> rot swap 2dup 2>R
    <= IF 2R>
        < IF 2DROP 0
        ELSE > THEN
    ELSE 2DROP -1 THEN
;

: d< rot swap 2dup 2>R
    >= IF 2R>
        > IF 2DROP 0
        ELSE < THEN
    ELSE 2DROP -1 THEN
;

: d= rot = -rot = and ;

( String/array handling words)

: NEWSTR ( -- adr)
  C_YARNBALL_ADR DUP
  @ DUP 0 SWAP !
  1 + SWAP !
;

: COUNT ( adr -- adr+1 strsize) DUP @ 1 -   SWAP 1 +   SWAP ;
: UNCOUNT ( adr+1 strsize -- adr) DROP 1 - ;

: s" [CHAR] " PARSE ;
: [s"] s" LIT, ; immediate
: ." s" COUNT TYPE ;

: [."] POSTPONE [s"]   PPW COUNT   PPW TYPE ; immediate
: ." STATE @ IF POSTPONE [."] ELSE ." THEN ; immediate allow_interpret
: s" STATE @ IF POSTPONE [s"] ELSE s" THEN ; immediate allow_interpret

\ Clears screen
: CLEAR 27 emit ." [1J" ;
: PAGE CLEAR 0 0 at-xy ;

: FILL ( c-char u char -- ) ROT ROT 0 ?DO 2DUP ! 1 + LOOP 2DROP ;
: MOVE ( addr1 addr2 u -- )
    ( copies u cells starting from address 1 intro address 2)
    0 ?DO
        ( addr1+i addr2+i )
        2DUP
        SWAP @
        SWAP !
        1 + SWAP 1 + SWAP
    LOOP
    2DROP
;
: CMOVE MOVE ;

: /STRING ( str n u -- str+u n-u )
    DUP -ROT - ( str u newsize)
    -ROT ( newsize str u)
    + SWAP
;
: ALLOT ( n -- )
        ( allots n cells of space in dictionary, )
        (  or release if negative)
    DUP 0 >= IF
        0 ?DO
            0 ,
        LOOP
    ELSE
        ( As of yet, this is unimplemented.)
        ( The HERE pointer is not the native way storage is represented in )
        ( nomforth, so there is currently no easy way to way to modify it either)
        EXIT
    THEN
;

: ARRAY ( size "name" --  ) CREATE ALLOT ;


: MARKER
    DP @ CREATE ,
    DOES> @ DP !
;

: ABORT ( string flag -- )
    IF
      ." EXCEPTION: "
      COUNT TYPE
      BYE
    ELSE
      DROP
    THEN
;
: ABORT" ( flag -- )
    POSTPONE s"
    POSTPONE LITERAL
    PPW SWAP
    PPW ABORT
; immediate


( [n stack items] n -- environment )
( like variable capture for closures )
( use 0 CAP to capture nothing. )
( DO NOT CAPTURE INSIDE A WORD DEFINITION )
( don't use : word [ 1 cap ] ;, note it's using cap)
( as an immediate, not inside a closure [ vs [: )
: CAP HERE SWAP  DUP ,   0 ?DO SWAP , LOOP ;

: PRINTARR
    DUP @ .
    [CHAR] L EMIT
    DUP @ DUP SWAP 0 ?DO
      2DUP i - + @ .
    LOOP
    2DROP
;

( pushes environment onto stack )
: PUSHARR
    DUP @ DUP SWAP 0 ?DO
      2DUP i - + @ -ROT
    LOOP
    2DROP
;

( DO NOT USE [: :] IN INTERPRET MODE)
( start anonymous function )
: [:
  1 STATE !
  HERE 4 + LIT,
  POSTPONE AHEAD
  0 ( dummy )
  0 ,  0 ,  0 ,
; immediate

( end anonymous function )
: :]
  DROP ( drop dummy )
  POSTPONE THEN
; immediate

( environment xt -- effect of xt with environment )
: func-exec
    >R
    pusharr
    R>
    execute
;

( example use: )
( : succ 1 cap [: 1 + :] ; )
( 4 succ func-exec )
( OUTPUT: 5 ok)

( Local registers code)
30 ARRAY lspA
VARIABLE lsp
( "locals stack pointer")
lspA lsp !

: LOCALS_FRAME_SIZE 3 ;

( If you look at this code, the first LOCALS_FRAME_SIZE)
( bytes of the locals stack are actually)
( never used. I guess they can be used)
( to store some metadata if you want)
lspA CONSTANT lsp_start

: lsp-> ( -- )  lsp @ LOCALS_FRAME_SIZE + lsp ! ;
: <-lsp ( -- )  lsp @ LOCALS_FRAME_SIZE - lsp ! ;

: >a ( n -- ) lsp @ 0 + ! ;
: >b ( n -- ) lsp @ 1 + ! ;
: >c ( n -- ) lsp @ 2 + ! ;

: a> ( -- n ) lsp @  0 + @ ;
: b> ( -- n ) lsp @  1 + @ ;
: c> ( -- n ) lsp @  2 + @ ;

( -- , starts local-mode word, puts the cleanup code in the return stack)
: ::
    :
    PPW lsp->
    ( substract 1 because the convention dictates the callee will increment it)
    ['] <-lsp >CODE 1 - LIT,
    PPW >r ( append continuation to return stack)
;

( word takes 1 parameter into A)
: ::1 :: PPW >a ;

( word takes 2 parameters into A B )
: ::2
      ::
      PPW >b
      PPW >a
;


( word takes 3 parameters into A B C)
: ::3
      ::
      PPW >c
      PPW >b
      PPW >a
;

( example usage that prints 2 1:)
( :: te 1 >c 2 . c> . ; )

: WORDS
    DP @
    BEGIN
        DUP 1 + @
        DUP 0 != IF COUNT TYPE BL EMIT THEN
        @
    DUP 0 = UNTIL
    DROP
;

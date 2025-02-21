( Nomagno's Forth complex standard words )
( Copyright 2025 Nomagno )
( MIT License )

: CELLS ;
: CELL 1 ;
: CELL+ 1 + ;
: -ROT ROT ROT ;
: NOOP ;
: COMPILE, , ;

: PICK ( x0 i*x u.i -- x0 i*x x0 )
  dup 0 = if drop dup exit then  swap >r 1 - recurse r> swap
;

: ROLL ( x0 i*x u.i -- i*x x0 )
  dup 0 = if drop exit then  swap >r 1 - recurse r> swap
;

: BETWEEN ( x1 x2 x3 -- B)
  ( if x1 is between x2 and x3, both included, leave -1 on the stack, else leave 0)
  >R
  2DUP >=
  NIP
  R> SWAP >R
  <=
  R> AND
;

( Control flow words )
: ?DO ( -- dest / D: l i -- , R: -- l i )
    PPW 2DUP
    PPW >
    POSTPONE IF
    POSTPONE BEGIN
    PPW 2>R
; immediate
: DO ( -- dest / D: l i -- , R: -- l i )
    POSTPONE BEGIN
    PPW 2>R
; immediate
: LOOP
    PPW 2R>
    1 LIT,
    PPW +
    PPW 2DUP
    PPW <=
    POSTPONE UNTIL
    POSTPONE THEN
    PPW 2DROP ( WE DROP FROM THE RETURN STACK BECAUSE IF WE )
              ( DID NOT LOOP, THE 2>R WAS NOT EXECUTED YET )
; immediate
: I r> r> DUP >r SWAP >r ; forbid_tco
: +I r> SWAP r> + >r >r ; forbid_tco

: CASE
    0 ( setup parameter with the amount of AHEAD references)
; IMMEDIATE
: GOF ( GENERAL OF, SIMPLY CONSUMES A FLAG)
    POSTPONE IF
    SWAP ( tuck OF reference behind reference count)
; immediate
: ROF ( x1 x2 x3 -- x1, if x2<=x1<=x3 execute, else skip to right after the closing ENDOF)
    PPW ROT
    PPW DUP
    PPW 2SWAP
    PPW BETWEEN
    POSTPONE GOF
    PPW DROP
; immediate
: OF ( x1 x2 -- x1, if x1==x2 execute, else skip to right after the closing ENDOF)
    PPW OVER
    PPW =
    POSTPONE GOF
    PPW DROP
; immediate
: ENDOF
    POSTPONE AHEAD ( create AHEAD reference to skip default case)
    ( OF_REF REF_COUNT AHEAD)
    -ROT ( AHEAD OF_REF REF_COUNT )
    SWAP ( AHEAD REF_COUNT OF_REF )
    POSTPONE THEN ( resolve last OF reference )
    1 + ( Increase reference count )
; immediate
: ENDCASE ( x -- )
    PPW DROP
    0 ?DO ( resolve all AHEAD references )
        POSTPONE THEN
    LOOP
; immediate

: ELSE ( orig1 -- orig2 / -- )
   ( resolve IF supplying alternate execution )
    POSTPONE AHEAD       ( unconditional forward branch orig2 )
    SWAP            ( put orig1 back on top )
    POSTPONE THEN       ( resolve forward branch from orig1 )
; immediate
: WHILE ( dest -- orig dest / flag -- )
    POSTPONE IF          ( conditional forward brach )
    SWAP           ( keep dest on top )
; immediate
: REPEAT ( orig dest -- / -- )
   ( resolve a single WHILE and return to BEGIN )
    POSTPONE AGAIN       ( uncond. backward branch to dest )
    POSTPONE THEN        ( resolve forward branch from orig )
; immediate

( Variable words)
: ISVAR ( xt -- is_it_a_properly_declared_variable) 2 + @ 28 rshift 8 and ;
: VARIABLE ( "name" --  ) CREATE 0 , ;
: CONSTANT ( n "name" --  )
    EMPTY_WORD
    LIT,
    C_T_E ,
;
: BEGIN-STRUCTURE ( -- addr 0 ; Exec -- size )
    CREATE
    HERE 0 0 , ( Mark stack, put down empty value )
    DOES> @   ( -- record-length )
;
: FIELD: ( addr n <"name"> -- addr ; Exec: addr -- 'addr )
    CREATE OVER , +
    DOES> @ +
;
: END-STRUCTURE ( addr n -- )
    SWAP !      ( set len )
;

( String/array handling words)

: NEWSTR ( -- adr)
  C_PAD_ADR DUP
  @ DUP 0 SWAP !
  1 + SWAP !
;

( adr -- , deletes all scratch pad entries starting from the selected address, by resetting the pad pointer)
: RESET_PAD_TO C_PAD_ADR ! ;

: "p [CHAR] " PARSE ;
: COUNT ( adr -- adr+1 strsize) DUP @ 1 -   SWAP 1 +   SWAP ;
: UNCOUNT ( adr+1 strsize -- adr) DROP 1 - ;
: ." "p DUP COUNT TYPE RESET_PAD_TO ;
: STRLIT"
    "p LIT,
    PPW COUNT
    PPW TYPE
; immediate
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

( [n stack items] n -- environment )
( like variable capture for closures )
( use 0 CAP to capture nothing. )
( DO NOT CAPTURE INSIDE A WORD DEFINITION )
( don't use : word [ 1 cap ] ;, note it's using cap
  as an immediate, not inside a closure [ vs [: )
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

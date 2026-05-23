( Nomagno's Forth simple standard words )
( Copyright 2025 Nomagno )
( MIT License )

( Build the regular forth language, smoothing out inconsistencies)

: ABC ;
: ACB SWAP ;
: BAC ROT SWAP ;
: BCA ROT ;
: CAB ROT ROT ;
: CBA SWAP ROT ;
: AXX 2DROP ;
: XBX ROT 2DROP ;
: XXC ROT ROT 2DROP ;
: -ROT CAB ;

: DICT_CURRENT ( -- current_dictionary_word_start)
    ( We can not do this because we do not have the word defined yet:)
    ( C_DICT_ADR @)
    ( So we hard-code it)
    31 @
;

: SET_HEADER_BIT ( idx -- , sets word bit)
    >R
    DICT_CURRENT 2 +
    DUP @
    1 R> LSHIFT    
    OR
    SWAP !
;

: MARK_VAR           ( -- ) 31 SET_HEADER_BIT ;
: FORBID_TCO         ( -- ) 30 SET_HEADER_BIT ;
( by default immediate words warn when being called directly from INTERPRET while STATE is 0)
: ALLOW_INTERPRET    ( -- ) 29 SET_HEADER_BIT ;
: IMMEDIATE          ( -- ) 28 SET_HEADER_BIT ;


: RECURSE DICT_CURRENT , ; immediate

( Ascii code for a blank space ' ')
: BL 32 ;

( Ascii code for a newline '\n')
: LF 10 ;

: LIT, 3 , , ;
: LITERAL LIT, ; immediate

: C_T_UNKNOWN 0 LIT, ; immediate
: C_T_NOP 1 LIT, ; immediate
: C_T_PRIM 2 LIT, ; immediate
: C_T_NUM 3 LIT, ; immediate
: C_T_RJMP 4 LIT, ; immediate
: C_T_RBJMP 5 LIT, ; immediate
: C_T_CRJMP 6 LIT, ; immediate
: C_T_CRBJMP 7 LIT, ; immediate
: C_T_ABSJMP 8 LIT, ; immediate
: C_T_L 9 LIT, ; immediate
: C_T_E 10 LIT, ; immediate
: C_T_E_NTC 11 LIT, ; immediate
: C_T_EXEC 12 LIT, ; immediate

: C_PC_ADR 1 LIT, ; immediate
: C_STATE_ADR 2 LIT, ; immediate
: C_STRPOS_ADR 3 LIT, ; immediate
: C_STRSIZE_ADR 4 LIT, ; immediate
: C_STRPTR_ADR 5 LIT, ; immediate
: C_FLAGS_ADR 6 LIT, ; immediate
: C_BASE_ADR 7 LIT, ; immediate
: C_EXP_ADR 8 LIT, ; immediate


: DECIMAL 10 C_BASE_ADR ! ;
: HEX 16 C_BASE_ADR ! ;
: OCTAL 16 C_BASE_ADR ! ;
: DEC DECIMAL ;
: BASE C_BASE_ADR ;
: EXP C_EXP_ADR ;

( When altering this, make sure to also alter the definition of DICT_CURRENT)
: C_DICT_ADR 31 LIT, ; immediate
: DP C_DICT_ADR ;

HEX
: C_YARNBALL_ADR 1A000 LIT, ; immediate
: USERMEM 16000 ;
DECIMAL

: BYE_DEFERRED
    C_FLAGS_ADR
    DUP @ 1 0 LSHIFT OR
    SWAP !
;

: ..SILENT
    C_FLAGS_ADR
    DUP @ 1 1 LSHIFT OR
    SWAP !
;

: ..VERBOSE
    C_FLAGS_ADR
    DUP @ 1 1 LSHIFT BITWISE_NOT AND
    SWAP !
;

: ..TRACE_ON
    C_FLAGS_ADR
    DUP @ 1 2 LSHIFT OR
    SWAP !
;

: ..TRACE_OFF
    C_FLAGS_ADR
    DUP @ 1 2 LSHIFT BITWISE_NOT AND
    SWAP !
;


: EXIT C_T_E , ; immediate
: EXIT_NTC C_T_E_NTC , ; immediate

: RAW_VAR_SIZE ( xt -- size)
    2 + @ 65535 AND
    3 +
;


: HERE ( -- next_empty_dictionary_byte)
    DICT_CURRENT
    DUP RAW_VAR_SIZE
    +
;

: IF ( Compile-time: -- orig, generates conditional branch, places forward ref on stack )
    HERE 1 +
    C_T_CRJMP ,
    0 ,
; immediate

 : AHEAD ( Compile-time: -- orig, generates unconditional branch, places forward ref on stack )
    HERE 1 +
    C_T_RJMP ,
    0 ,
; immediate

 : GOTO ( Compile-time: loc -- , generates unconditional absolute jump that reads FROM THE DATA STACK)
    C_T_ABSJMP ,
    ,
; immediate

: EXECUTE_PRIM ( Compile-time: -- , generates execute primitive, at run-time it'll execute an xt on the data stack)
    C_T_EXEC ,
; immediate

: EXECUTE
    EXECUTE_PRIM
;

: THEN ( Compile-time: orig --, resolves forward ref )
    DUP
    HERE SWAP 1 - -
    SWAP !
; immediate

: BEGIN ( Compile-time: -- dest, generates back ref )
    HERE
; immediate

: AGAIN ( Compile-time: dest -- , resolves back ref with unconditional branch )
    C_T_RBJMP ,
    0 ,
    HERE 1 -
    DUP 1 -
    ROT -
    SWAP !
; immediate

: UNTIL ( Compile-time: dest -- , resolves back ref with conditional branch )
    C_T_CRBJMP ,
    0 ,
    HERE 1 -
    DUP 1 -
    ROT -
    SWAP !
; immediate

: >NAME ( xt -- xt_name_addr )
    1 + @
;
: >SIZE ( xt -- size of usable variable body assuming it was initialized with VARIABLE)
    RAW_VAR_SIZE 7 -
;
: >CODE ( xt -- first address of executable code)
    3 +
;

: >BODY ( xt -- body_start_addr)
    DUP >SIZE 0 >= IF ( If "standard" size is negative it's definitely not a regular variable)
        7 +
        EXIT
    THEN
    ( Else: Makes no sense to call >BODY on such a word, return 0 as invalid )
    DROP 0
;

: STATE C_STATE_ADR ;
: [ C_STATE_ADR 0 SWAP ! ; immediate
: ] C_STATE_ADR 1 SWAP ! ;

: ' ( "name" -- xt ) BL WORD FIND 
    DUP 0 = IF ( if the word doesn't exist, return 0)
        2DROP 0
    THEN
    DUP 0 != IF
        DROP
    THEN
;

: ['] ( compilation: "name" --; run-time: -- xt ) ' LIT, ; immediate
: PPW
    ' LIT,
    ['] , ,
; immediate

( Control flow words )
: ?DO ( -- dest / D: l i -- , R: -- l i )
    PPW 2DUP
    PPW >
    POSTPONE IF
    POSTPONE BEGIN
    PPW 2>R
; immediate
: DO ( -- dest / D: l i -- , R: -- l i )
    1 LIT,
    POSTPONE IF
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
: +LOOP
    PPW 2R>
    PPW ROT
    PPW +
    PPW 2DUP
    PPW <=
    POSTPONE UNTIL
    POSTPONE THEN
    PPW 2DROP ( WE DROP FROM THE RETURN STACK BECAUSE IF WE )
              ( DID NOT LOOP, THE 2>R WAS NOT EXECUTED YET )
; immediate
: UNLOOP  PPW 2R>  PPW 2DROP ; immediate
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

: TRUE -1 ;
: FALSE 0 ;

( Variable words)
: ISVAR ( xt -- is_it_a_properly_declared_variable) 2 + @ 28 rshift 8 and ;
: VARIABLE ( "name" --  ) CREATE 0 , ;
: CONSTANT ( n "name" --  )
    EMPTY_WORD
    LIT,
    C_T_E ,
;


( Make sure to initialize the heap)
HEAP_INIT

\ This is the end of the bootstrapping, now we build some utility functions.


: EVALUATE SAVE-INPUT INTERPRET RESTORE-INPUT ;

( -- n, where n is 0 if an invalid character was entered,
(       and a code 1-26 if a lowercase alphabet letter was entered)
: GETLETTER GETC DUP DUP [CHAR] a >= SWAP [CHAR] z <= AND IF [char] a - 1 + ELSE DROP 0 THEN ;
: GETNUMBER GETN ;


: BETWEEN ( x1 x2 x3 -- B)
  ( if x1 is between x2 and x3, both included, leave -1 on the stack, else leave 0)
  >R
  2DUP >=
  NIP
  R> SWAP >R
  <=
  R> AND
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

: BOOL_NORM LOGICAL_NOT LOGICAL_NOT ;


\ This section exists for standard forth compatibility
: <> != ;
: 2/ 2 / ;
: 1+ 1 + ;
: 1- 1 - ;
: 0= 0 = ;
: c! ! ;
: c@ @ ;
: invert bitwise_not ;
: r@ r>  rdup r>  swap >r ; forbid_tco
: char+ 1 + ;
: u< < ;
: invert BITWISE_NOT ;
: negate ARITHMETICAL_NOT ;

: CELLS ;
: CELL 1 ;
: CELL+ 1 + ;
: NOOP ;
: COMPILE, , ;
: ALIGN ;


: PICK ( x0 i*x u.i -- x0 i*x x0 )
  dup 0 = if drop dup exit then  swap >r 1 - recurse r> swap
;

: ROLL ( x0 i*x u.i -- i*x x0 )
  dup 0 = if drop exit then  swap >r 1 - recurse r> swap
;

( adr -- , deletes all string sections entries starting from the selected address, by resetting the string section pointer)
: RESET_YARNBALL_TO C_YARNBALL_ADR ! ;

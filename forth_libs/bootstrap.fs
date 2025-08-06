( Nomagno's Forth simple standard words )
( Copyright 2025 Nomagno )
( MIT License )

( Build the regular forth language, smoothing out inconsistencies)
( Note this only builds a barebones core, also include utilities.fs for a usable language)

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
: C_PAD_ADR 1A000 LIT, ; immediate
: USERMEM 16000 ;
DECIMAL

: QUIT BYE ;

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
: ] C_STATE_ADR -1 SWAP ! ;

: ' ( "name" -- xt ) BL WORD FIND 
    DUP 0 = IF ( if the word doesn't exist, return 0)
        DROP DROP 0
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

( Make sure to initialize the heap)
HEAP_INIT

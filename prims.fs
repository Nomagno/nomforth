( Nomagno's Forth simple standard words )
( Copyright 2025 Nomagno )
( MIT License )

: DICT_CURRENT ( -- current_dictionary_word_start)
    ( We can not do this because we do not have the word defined yet:)
    ( C_DICT_POINTER_ADDRESS_CONST @)
    ( So we hard-code it)
    31 @
;

: IMMEDIATE ( -- )
    DICT_CURRENT 2 +
    DUP @
    1 24 LSHIFT    
    OR
    SWAP !
;

: FORBID_TCO ( -- )
    DICT_CURRENT 2 +
    DUP @
    1 30 LSHIFT    
    OR
    SWAP !
;


: RECURSE DICT_CURRENT , ; immediate

( Ascii code for a blank space ' ')
: BL 32 ;

( Ascii code for a newline '\n')
: LF 10 ;

: LIT, 3 , , ;
: LITERAL LIT, LITERAL ; immediate

: C_T_UNKNOWN_CONST 0 LIT, ; immediate
: C_T_NOP_CONST 1 LIT, ; immediate
: C_T_PRIM_CONST 2 LIT, ; immediate
: C_T_NUM_CONST 3 LIT, ; immediate
: C_T_RELJUMP_CONST 4 LIT, ; immediate
: C_T_RELJUMPBACK_CONST 5 LIT, ; immediate
: C_T_CONDRELJUMP_CONST 6 LIT, ; immediate
: C_T_CONDRELJUMPBACK_CONST 7 LIT, ; immediate
: C_T_ABSJUMP_CONST 8 LIT, ; immediate
: C_T_LEAVELABEL_CONST 9 LIT, ; immediate
: C_T_END_CONST 10 LIT, ; immediate
: C_T_END_NOTAILCALL_CONST 11 LIT, ; immediate

: C_PROGRAM_COUNTER_ADDRESS_CONST 1 LIT, ; immediate
: C_COMPILE_STATE_ADDRESS_CONST 2 LIT, ; immediate

( When altering this, make sure to also alter the definition of DICT_CURRENT)
: C_DICT_POINTER_ADDRESS_CONST 31 LIT, ; immediate


: EXIT C_T_END_CONST , ; immediate
: EXIT_NTC C_T_END_NOTAILCALL_CONST , ; immediate

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
    C_T_CONDRELJUMP_CONST ,
    0 ,
; immediate

 : AHEAD ( Compile-time: -- orig, generates unconditional branch, places forward ref on stack )
    HERE 1 +
    C_T_RELJUMP_CONST ,
    0 ,
; immediate

: THEN ( Compile-time: orig --, resolves forward ref )
    DUP
    HERE SWAP 1 - -
    SWAP !
; immediate

: BEGIN ( Compile-time: -- dest, generates back ref )
    HERE
; immediate

: AGAIN ( Compile-time: dest -- , resolves back ref with unconditional branch )
    C_T_RELJUMPBACK_CONST ,
    0 ,
    HERE 1 -
    DUP 1 -
    ROT -
    SWAP !
; immediate

: UNTIL ( Compile-time: dest -- , resolves back ref with conditional branch )
    C_T_CONDRELJUMPBACK_CONST ,
    0 ,
    HERE 1 -
    DUP 1 -
    ROT -
    SWAP !
; immediate

: WHO ( xt -- xt_name_addr )
    1 + @
;
: >SIZE ( xt -- size of usable variable body assuming it was initialized with VARIABLE)
    RAW_VAR_SIZE 7 -
;

: >BODY ( xt -- body_start_addr)
    DUP >SIZE >= 0 IF ( If "standard" size is negative it's definitely not a regular variable)
        7 +
    THEN
    DUP >SIZE < 0 IF ( Makes no sense to call >BODY on such a word, return 0 as invalid )
        DROP 0
    THEN
;

: STATE C_COMPILE_STATE_ADDRESS_CONST @ ;
: [ C_COMPILE_STATE_ADDRESS_CONST 0 SWAP @ ; immediate
: ] C_COMPILE_STATE_ADDRESS_CONST 1 SWAP ! ;

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

: COUNT_DIGITS ( x -- number_of_digits_of_x )
    1 SWAP
    BEGIN DUP 9 > WHILE
        10 /
        SWAP 1 + SWAP
    REPEAT
    DROP
;

: PRINTNEXT ( adr -- , prints values pointed to by next cell) 1 + @ . ;
: PRINTWORD ( xt -- , prints name of represented by xt) WHO COUNT TYPE ;

: TABS 2 * SPACES ;
: GEO_RECIP ( n m -- x )
    2DUP < IF SWAP - ELSE 2DROP 0 THEN
    ( x is either 0 if there is no space, or m-n if there is space.
    ( m is the "maximum", and n is the "space taken up from that maximum".)
    ( Useful for computing the amount of negative space in 1 dimension, )
    ( like when you want to add padding to make a piece of text fixed-width )
;

: DASM_GET_VSTART 2 + 2 + @ ;
: DASM_GET_VJUMP 2 + 3 + @ ;
: DASM_GET_VCODE 2 + 4 + @ ;

: XT-SEE ( xt -- )
    DUP 0 = IF
        STRLIT" WORD NOT FOUND"
        DROP
        EXIT
    THEN

    CR
    0 SWAP ( set-up indentation )

    DUP ISVAR IF
        STRLIT" Variable " DUP PRINTWORD
        STRLIT" , data size" DUP >SIZE . CR

        STRLIT" Data starts at address: 0x"
        DUP DASM_GET_VSTART X. CR

        1 TABS
        DUP DASM_GET_VJUMP CASE
            C_T_END_CONST     OF STRLIT" No special execution behaviour has been defined" CR ENDOF
            C_T_ABSJUMP_CONST OF STRLIT" Code at address 0x" DUP DASM_GET_VCODE X. CR ENDOF
            STRLIT" This variable is not following the standard variable format, weird" CR
        ENDCASE

        1 TABS
        STRLIT" Data dump of var:" CR

        DUP >SIZE 0 ?DO
            2 TABS
            DUP I  4 +  3 +   + ( get ivar and add it to current XT )
            STRLIT" 0x" @ X. CR
        LOOP
    ELSE
        STRLIT" : "
        DUP WHO COUNT TYPE CR
        DUP RAW_VAR_SIZE 3 - 0 ?DO
            1 TABS
            STRLIT" ["
                I .
                I COUNT_DIGITS 4 GEO_RECIP SPACES ( Pad up to 4 spaces )
            STRLIT" ]"
            SWAP DUP TABS SWAP ( Appropiate indentation level for actual code shown)

            DUP I 3 + + ( get ivar and add it to current XT )
            DUP @ CASE
                C_T_UNKNOWN_CONST         OF STRLIT" UNKNOWN" CR ENDOF
                C_T_NOP_CONST             OF STRLIT" NO-OP" CR ENDOF
                C_T_PRIM_CONST            OF 1 +i STRLIT" PRIMITIVE:" DUP PRINTNEXT CR ENDOF
                C_T_NUM_CONST             OF 1 +i STRLIT" NUMBER:" DUP PRINTNEXT CR ENDOF
                C_T_RELJUMP_CONST         OF 1 +i STRLIT" JUMP FORWARD BY:" DUP PRINTNEXT CR ENDOF
                C_T_RELJUMPBACK_CONST     OF 1 +i STRLIT" JUMP BACKWARD BY:" DUP PRINTNEXT CR ENDOF
                C_T_CONDRELJUMP_CONST     OF 1 +i STRLIT" COND JUMP FORWARD BY:" DUP PRINTNEXT CR ENDOF
                C_T_CONDRELJUMPBACK_CONST OF 1 +i STRLIT" COND JUMP FORWARD BY:" DUP PRINTNEXT CR ENDOF
                C_T_ABSJUMP_CONST         OF 1 +i STRLIT" ABSOLUTE JUMP TO:" DUP PRINTNEXT CR ENDOF
                C_T_LEAVELABEL_CONST      OF STRLIT" LEAVE LABEL, THIS IS PROBABLY AN ERROR/BUG" CR ENDOF
                C_T_END_CONST             OF STRLIT" RETURN" CR ENDOF
                C_T_END_NOTAILCALL_CONST  OF STRLIT" RETURN (no tail recursion allowed)" CR ENDOF
                DUP STRLIT" WORD: " PRINTWORD CR
            ENDCASE
            DROP
        LOOP
    THEN
    2DROP
;

: SEE ( "<name>" -- ) ' XT-SEE ;

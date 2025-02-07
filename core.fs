\ Nomagno's Forth composite standard words
\ Copyright (c) 2025 Nomagno
\ MIT License


: WHILE ( dest -- orig dest / flag -- )
    POSTPONE IF          ( conditional forward brach )
    SWAP           ( keep dest on top )
; immediate
: REPEAT ( orig dest -- / -- )
   ( resolve a single WHILE and return to BEGIN )
    POSTPONE AGAIN       ( uncond. backward branch to dest )
    POSTPONE THEN        ( resolve forward branch from orig )
; immediate
: ELSE ( orig1 -- orig2 / -- )
   ( resolve IF supplying alternate execution )
    POSTPONE AHEAD       ( unconditional forward branch orig2 )
    SWAP            ( put orig1 back on top )
    POSTPONE THEN       ( resolve forward branch from orig1 )
; immediate


: ' ( "name" -- xt ) BL WORD FIND 
    DUP 0 = IF ( if the word doesn't exist, return 0)
        DROP DROP 0
    ELSE
        DROP
    THEN
;

: ['] ( compilation: "name" --; run-time: -- xt ) ' LIT, ; immediate
: PPW
    ' LIT,
    ['] , ,
; immediate

: LITERAL LIT, ; immediate

: CREATE 0 VARIABLE ;

: CELLS ;
: CELL 1 ;
: CELL+ 1 + ;
: NOOP ;

: /STRING ( str n u -- str+u n-u )
    DUP ROT ROT - ( str u newsize)
    ROT ROT ( newsize str u)
    + SWAP
;

: "p [CHAR] " PARSE ;
: ." "p TYPE ;
: STRLIT"
    "p DROP 1 - LIT,
    PPW COUNT
    PPW TYPE
; immediate

: CONSTANT >R
    :
    R>
    POSTPONE LITERAL
    POSTPONE ;
;


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
    PPW 2DROP ( WE DROP FROM THE RETURN STACK BECAUSE IF WE )
              ( DID NOT LOOP, THE 2>R WAS NOT EXECUTED YET )
; immediate

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

: I r> r> DUP >r SWAP >r ;
: +I r> SWAP r> + >r >r ;

: COUNT_DIGITS ( x -- number_of_digits_of_x )
    1 SWAP
    BEGIN DUP 9 > WHILE
        10 /
        SWAP 1 + SWAP
    REPEAT
    DROP
;

: ISVAR ( xt -- is_it_a_properly_declared_variable) 2 + @ 28 rshift 8 and ;

( This XT-SEE is the worst forth I have ever seen or written, pending a refactoring)
: XT-SEE ( xt -- )
    DUP 0 = IF
        STRLIT" WORD NOT FOUND"
        EXIT
    THEN

    CR
    0 SWAP ( set-up indentation )

    DUP ISVAR IF
        STRLIT" Variable "
        DUP WHO COUNT TYPE
        STRLIT" , data size" DUP >SIZE . CR
        STRLIT" Data starts at address: 0x"
        DUP 2 +  2 + @ X. CR
        DUP 2 +  3 + @ 10 = IF
            2 SPACES
            STRLIT" No special execution behaviour has been defined" CR
        THEN
        DUP 2 +  3 + @ 8 = IF
            2 SPACES
            STRLIT" Code at address 0x"
            DUP 2 +  4 + @ X. CR
        THEN

        2 SPACES
        STRLIT" Data dump of var:" CR

        DUP RAW_VAR_SIZE 4 DO
            4 SPACES
            DUP I 3 + + ( get ivar and add it to current XT )
            STRLIT" 0x" @ X. CR
        LOOP
    ELSE
        STRLIT" : "
        DUP WHO COUNT TYPE CR
        DUP RAW_VAR_SIZE 0 DO
            2 SPACES
            STRLIT" ["
            I
            DUP .
            COUNT_DIGITS DUP 4 < IF 4 SWAP - SPACES ELSE 0 SPACES THEN
            STRLIT" ]"
            SWAP DUP 2 * SPACES SWAP ( Appropiate indentation level )

            DUP I 3 + + ( get ivar and add it to current XT )
            DUP @ 0 = IF
                STRLIT" UNKNOWN" CR
            THEN
            DUP @ 1 = IF
                STRLIT" NO-OP" CR
            THEN
            DUP @ 2 = IF
                STRLIT" PRIMITIVE:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 3 = IF
                STRLIT" NUMBER:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 4 = IF
                STRLIT" JUMP FORWARD BY:"
                1 +i
                DUP 1 + @ .
                CR
                ( ROT 1 + ROT ROT \increment indentation level )
            THEN
            DUP @ 5 = IF
                STRLIT" JUMP BACKWARD BY:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 6 = IF
                STRLIT" COND JUMP FORWARD BY:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 7 = IF
                STRLIT" COND JUMP BACKWARD BY:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 8 = IF
                STRLIT" ABSOLUTE JUMP TO:"
                1 +i
                DUP 1 + @ .
                CR
            THEN
            DUP @ 9 = IF
                STRLIT" LEAVE LABEL [IF YOU'RE SEEING THIS IT'S PROBABLY AN ERROR]" CR
            THEN
            DUP @ 10 = IF
                STRLIT" RETURN" CR
            THEN
            DUP @ 11 = IF
                STRLIT" RETURN (no tail recursion allowed)" CR
            THEN
            DUP @ 12 >= IF
                STRLIT" WORD: "
                DUP @ WHO COUNT TYPE
                CR
            THEN
            DROP
        LOOP
    THEN
;

: SEE ( "<name>" -- ) ' XT-SEE ;

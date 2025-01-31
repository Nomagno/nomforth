\ Nomagno's Forth composite standard words
\ Copyright (c) 2025 Nomagno
\ MIT License


: ' ( "name" -- xt ) BL WORD FIND DROP ;
: ['] ( compilation: "name" --; run-time: -- xt ) ' LIT, ; immediate
: PPW
    POSTPONE [']
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

: CONSTANT >R
    :
    R>
    POSTPONE LITERAL
    POSTPONE ;
;

: WHILE ( dest -- orig dest / flag -- )
    POSTPONE IF          \ conditional forward brach
    SWAP           \ keep dest on top
; immediate
: REPEAT ( orig dest -- / -- )
   \ resolve a single WHILE and return to BEGIN
    POSTPONE AGAIN       \ uncond. backward branch to dest
    POSTPONE THEN       \ resolve forward branch from orig
; immediate
: ELSE ( orig1 -- orig2 / -- )
   \ resolve IF supplying alternate execution
    POSTPONE AHEAD       \ unconditional forward branch orig2
    SWAP            \ put orig1 back on top
    POSTPONE THEN       \ resolve forward branch from orig1
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
    PPW 2DROP \ WE DROP FROM THE RETURN STACK BECAUSE IF WE
              \ DID NOT LOOP, THE 2>R WAS NOT EXECUTED YET
; immediate

: BEGIN-STRUCTURE ( -- addr 0 ; Exec -- size )
    CREATE
    HERE 0 0 , \ Mark stack, put down empty value
    DOES> @   \ -- record-length
;
: FIELD: ( addr n <"name"> -- addr ; Exec: addr -- 'addr )
    CREATE OVER , +
    DOES> @ +
;
: END-STRUCTURE ( addr n -- )
    SWAP !      \ set len
;

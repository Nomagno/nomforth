\ Welcome to the forth menu library
\ Copyright Nomagno 2025, MIT license
\ This is an optimized port
\ to nomforth of my own standard forth
\ library:
\ https://gist.github.com/Nomagno/e6f7236e7da173bd5beb35eac0650841

: sp" ( ccc<"name"> -- adr, adr is a counted string stored persistently )
   [CHAR] " PARSE
;

: 0, 0 , ;
: 1+ 1 + ;
: 1- 1 - ;
: 0SWAP 0 SWAP ;


DEFER HANDLE ( Forward reference to HANDLE, so that we can define it later for comfort )

: NAMED_MENU ( -- last_adr ) HERE DUP 0, CONSTANT ;
: ANON_MENU ( -- menu_adr last_adr ) HERE DUP 0, ;

: LABEL" ( ccc<"Label text"> last_adr -- new_last_adr )
    sp" SWAP  ( consume quoted string)
    HERE SWAP ! ( store current address in next field)
      ,   ( store quoted string)
    0,   ( store meta tag)
    0,   ( store does nothing tag)
    0,   ( make space for next field)
    HERE 1- ( new last address)
;

: ACTION" ( ccc<"Label text"> last_adr xt -- new_last_adr )
    sp" ROT  ( consume quoted string)
    HERE SWAP ! ( store current address in next field)
      ,   ( store quoted string)
      ,   ( store code)
    0,   ( store empty data)
    0,   ( make space for next field)
    HERE 1- ( new last address)
;

: SUBMENU" ( ccc<"Label text"> last_adr submenu_adr -- new_last_adr )
    sp" ROT  ( consume quoted string)
    HERE SWAP ! ( store current address in next field)
      ,   ( store quoted string)
    ['] HANDLE ,   ( store HANDLE adr)
      ,   ( store submenu adr)
    0,   ( make space for next field)
    HERE 1- ( new last address)
;

sp" GO BACK" CONSTANT EXIT_TEXT
: END_MENU ( last_adr -- )
    HERE SWAP ! ( store current address in next field)
    EXIT_TEXT , ( store string)
    0,   ( store meta tag)
    1 ,   ( store exit tag)
    0,   ( close the menu, no next field)
;

: NEXT_ITEM ( current_item_adr -- next_item_adr ) 3 + @ ;

: DISPLAY ( position menu_adr -- )
    CR
    @ ( access the first field pointer, if 0 the menu is empty )
    DUP 0 = IF STRLIT" Empty menu!!!" 2DROP EXIT CR THEN
    0 >R ( position tracker on the return stack )
    BEGIN ( loop )
        SWAP DUP  R> DUP >R = IF ( Render cursor if position tracker matches position)
            STRLIT" ->"
        ELSE
            STRLIT"   "
        THEN
        SWAP
        DUP @ COUNT TYPE CR ( text field)
            R> 1+ >R ( increment position tracker )
        NEXT_ITEM
     DUP 0 = UNTIL ( until the next field is 0)
    R> DROP
    2DROP ( consume our parameters )
;

VARIABLE INPUT_BUFFER
: INPUT
    BEGIN
        CR STRLIT" U/D/C? "
        INPUT_BUFFER 1 ACCEPT ( buffer size -- read_amount )
        1 = ( flag to check if the input was valid)
        DUP IF
            INPUT_BUFFER @ CASE
                ( Valid cases: set flag to 1)
                [CHAR] U OF 0SWAP  1 OR  ENDOF
                [CHAR] D OF 1 SWAP  1 OR  ENDOF
                [CHAR] C OF 2 SWAP  1 OR  ENDOF
                ( Default case: ) DROP 0 AND
                ( set flag as invalid )
                0 ( dummy to be discarded by endcase)
            ENDCASE
        THEN
        ( If the input was invalid, loop)
    UNTIL
;

: INDEX ( menu_adr idx -- menu_item[idx]_adr, menu_item[idx]_adr will be 0 if the idx'th element of `menu' does not exist )
    SWAP @
    DUP 0 = IF  2DROP 0 EXIT  THEN ( if the menu is empty, return address 0 as no index is valid )
    SWAP 0 ?DO
        NEXT_ITEM DUP 0 = IF
            UNLOOP EXIT ( leave 0 on the stack)
        THEN
    LOOP
;

: UPDATE ( position menu_adr input -- ... finish_status new_position)
    ROT SWAP ( menu_adr position input )
    CASE ( menu_adr position )
        0 ( up) OF
            DUP 0 = IF
                2DROP 0 0 ( 0 0)
            ELSE
                1- SWAP DROP 0SWAP ( 0 position-1)
            THEN
        ENDOF
        1 ( down) OF
            DUP -ROT 1+ ( position menu_adr position+1 ) INDEX
            ( position element_at_position+1)
            0 = IF ( case: if the element is 0, it doesn't exist, so we can't go down )
                0SWAP ( case: leave 0 position on stack)
            ELSE ( case: else just go down)
                0SWAP 1+ ( leave 0 position+1 on stack)
            THEN
        ENDOF
        2 ( case: confirm) OF
            DUP -ROT ( position menu_adr position ) INDEX
            ( position element_at_position)
            1+ ( position meta/code_adr )
            DUP @ 0 = IF ( case: meta )
                1+ ( position metadata_adr)
                @ 0 = IF
                    0SWAP ( case: do nothing )
                ELSE
                    1 SWAP ( case: exit )
                THEN
            ELSE ( case: code )
                DUP 1+ ( position code_adr argument_adr)
                ROT >R ( important: store position in return stack, as execute might mangle our stack)
                @ DUP 0 <> IF ( case: do pass the argument, as it has value != 0)
                    SWAP @ ( argument xt, for submenus mostly)
                    EXECUTE
                ELSE ( case: don't pass any arguments)
                    DROP @ ( xt)
                    EXECUTE
                THEN
                0 R> ( 0 position)
            THEN
        ENDOF
        STRLIT" ERROR: INVALID INPUT TO MENU UPDATE" BYE
    ENDCASE
;

( We will vector this XT into the previously DEFER'ed HANDLE)
:noname ( menu_adr -- )
    0SWAP ( Initial position)
    BEGIN
        2DUP DISPLAY ( position menu_adr -- )
        DUP >R ( copy menu_adr to the return stack since it'll be consumed)
        INPUT UPDATE ( position menu_adr input -- finish_status new_position)
        R> ( recover menu_adr)
    ROT 1 = UNTIL ( Loop until the exit status is 1 )
    2DROP ( leave nothing on the stack )
; IS HANDLE

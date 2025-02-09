( Nomagno's Forth example )
( Copyright 2025 Nomagno )
( MIT License )

\ Make sure to first load core.fs
\ A nice way to test the program is simply to do:
\ $ cat core.fs example.fs | /path/to/executable

: factR ( n -- n! )
    DUP 0 = IF
        DROP 1
    ELSE
        DUP 1 - RECURSE * ( Recular recursion )
    THEN
;

: fact_tail_rec ( n n!_cumulative [pass 1] -- 0 n! )
    SWAP DUP 0 = IF
        DROP EXIT
    ELSE
        DUP 1 - SWAP ROT * RECURSE ( Tail recursion)
    THEN
;

: factTR ( n -- n! )
    1 fact_tail_rec
;

\ Maximum representable factorial in signed 32 bits: 12! == 479001600.

\ To test out that it's really doing tail recursion, a number higher than at least 290 crashes the interpreter with
\ factR, but works "just fine" (terminates with the expected behaviour when repeated overflow is considered,
\ AKA returns 0) with factTR.

bl WORD Starting_recursion_now count type CR \ To show the string Starting_recursion_now on screen
12 factR .
12 factTR .

\ Structure usage, compliant with the Forth Standard
BEGIN-STRUCTURE Vec3 \ A vec3 of *double*-sized integers
    2 FIELD: V.x
    2 FIELD: V.y
    2 FIELD: V.z
END-STRUCTURE

Vec3 VARIABLE MyVec

\ We set the first byte of each field of this 3D vector
9 MyVec V.x !
99 MyVec V.y !
999 MyVec V.z !

\ We then retrieve it, this should print "9 99 999 OK"
MyVec V.x @ .
MyVec V.y @ .
MyVec V.z @ .

quit

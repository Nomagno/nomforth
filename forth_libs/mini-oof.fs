\ Port of Forth's mini-OOF
\ Removed +cell etc as it's not needed in nomforth, renamed :: -> super because of a conflict, renamed defines -> implements

: 2@ DUP 1+ @ SWAP @ ;

: method ( m v "name" -- m' v )
    create over , \ stores the m offset to "name"
    swap 1+ swap  \ increases the m offset
DOES> ( ... o -- ... )
    @ over @ + @ execute \ when called, executes the method at the offset
;

: var ( m v size "name" -- m v' )
    create  over , \ stores the v offset to "name"
    + \ increases the v offset by "size"
DOES> ( o -- addr )
    @ + \ when called, fetches the address of the member variable at the offset
;

: class ( class -- class methods vars )
    dup 2@ \ an object starts with the offset where is methods end, and the offset where its vars end
;

: end-class  ( class methods vars "name" -- )
  create
     here >r
  , \ stores the vars offset to "name"
  dup , \ stores the the methods offset to "name"
    2 ?DO
        ['] noop , \ store a no-op code vector for each method
    LOOP
  1+
  dup 1+
  r>
  rot @ 2 /string move ;


\ Self explanatory, alloc
: implements ( xt class "name" -- ) ' >body @ + ! ;

\ Self explanatory, allocates an object with the correct size for vars
: new ( class -- o )  here over @ allot swap over ! ;

\ :noname [ parentClass super parentClassMethod ] ; childClass implements childClassMethod
\ so basically it's like Java's 'super' keyword
: super ( class "name" -- ) ' >body @ + @ compile, ;

create Root  1 , 2 ,

Root class
  method init
end-class Object

\ By convention, we assume that classes will ALWAYS have an init method
: new: ( .. o "name" -- )
    new dup constant init ;


\ Object class
\   method draw
\ end-class Graphical ( "Object" is the parent class )

\ Graphical class
\   cell var circle-radius
\ end-class Circle ( "Graphical" is the parent class )

\ : draw-circle ( x y -- )
\     at-xy
\ [CHAR] O emit ;

\ :noname ( r -- )
\   circle-radius ! ; Circle implements init

\ :noname ( x y -- )
\   page
\       circle-radius @
\       draw-circle
\   cr
\ ; Circle implements draw


\ ( Equivalent to: )
\ ( Circle new constant my-circle )
\ ( 5 my-circle init )
\ 5 Circle new: my-circle
\ 12 5 my-circle draw

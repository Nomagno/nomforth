hex
: NAN 7FFFFFFF ;
: Infinity 7F800000 ;
: -Infinity FF800000 ;
dec

: FS 10 f.ss CR ;

:: quadratic ( a b c -- M K, such that M+K and M-K are both roots of ax^2 + bx + c)
  >C >B >A
  -4f A> C> f* f*

  B> B> f*
  f+
  DUP
  0.0f f< IF
    DROP
    NaN NaN
  ELSE
    fSQRT
    -1f B> f*
    2f A> f* >D
    D> f/
    SWAP
    D> f/
  THEN
;

: printQuadraticSolutions 2DUP f+ -ROT f- f. f. CR ;

3f 8f 4f quadratic
printQuadraticSolutions

4f 8f 4f quadratic
printQuadraticSolutions

7f 8f 4f quadratic
printQuadraticSolutions

0f 8f 4f quadratic
printQuadraticSolutions

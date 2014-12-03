%% gecode-string:	bounded-length string constraints for Gecode
%% url: 		github.com/jossco/gecode-string
%% author: 	Joseph D. Scott, Uppsala University
%% ========
%% file: 		./minizinc/mznlib/gecode-string-open.mzn
%% version: 	0.2.1
%% date: 		Wed Dec  3 17:58:46 CET 2014
%% ========
%% Predicates for gecode-string bounded-length string constraints
%% Generates flatzinc recognized by fzn-gecode-string (NOT fzn-gecode)

int: pad_char = 96;

predicate str_pad(array[int] of var int: X, var int: lenX);
% =
%  forall(i in index_set(X))(i>lenX <-> X[i]==pad_char);

%% EQUAL:
%predicate str_equal_d(array[int] of var int: X, var int: lenX, array[int] of var% int: Y, var int: lenY) =
%  lenX == lenY
%  /\ forall(i in index_set(X))(i <= lenX -> X[i] == Y[i]);
%% EQUAL:
predicate str_equal(array[int] of var int: X, var int: lenX, array[int] of var int: Y, var int: lenY);
predicate str_equal_reif(array[int] of var int: X, var int: lenX, array[int] of var int: Y, var int: lenY) =
  lenX == lenY
  /\ forall(i in index_set(X))(i <= lenX -> X[i] == Y[i]);


%% CONCAT:
predicate str_concat_d(array[int] of var int: X, var int: lenX, array[int] of var int: Y, var int: lenY, array[int] of var int: Z, var int: lenZ) = 
  lenZ == lenX + lenY
  /\ forall(i in index_set(X))((i <= lenX) -> (X[i] == Z[i]))
  /\ forall(i in index_set(X) union {0},j in index_set(Y))((i==lenX /\ j <= lenY) -> (Z[i+j]==Y[j]));
%% CONCAT:
predicate str_concat(array[int] of var int: X, var int: lenX, array[int] of var int: Y, var int: lenY, array[int] of var int: Z, var int: lenZ);  
  
  
%% NEQUAL:
predicate str_nequal(array[int] of var int: X, var int: lenX, array[int] of var int: Y, var int: lenY) =
  not ((lenX == lenY) /\ forall(i in index_set(X))((i <= lenX /\ i <= lenY) -> X[i] == Y[i]));
  


%% REGULAR:
predicate str_regular(array[int] of var int: X, var int: lenX, int: Q, int: S, array[int,int] of int:d, int:q0, set of int: F);
%% REGULAR:
predicate str_regular_d(array[int] of var int: X, var int: lenX, int: Q, int: S, array[int,int] of int:d, int:q0, set of int: F) =
  let { int: Q2=Q+1,
        set of int: F2 = F union{Q2},
        array[1..Q2,1..S] of int: d2 =
          array2d(1..Q2,1..S,
            [if i in 1..Q
             then
               if j == pad_char
               then
                 if i in F
                 then Q+1
                 else 0
                 endif
               else
                 d[i,j]
               endif
             else
               if j == pad_char
               then i
               else 0
               endif
             endif
           | i in 1..Q2, j in 1..S]
          )
        } in 
  regular(X, Q2, S, d2, q0, F2)

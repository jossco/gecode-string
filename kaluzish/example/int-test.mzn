include "gecode-string-ascii.mzn";
include "gecode-string-pad.mzn";

int: wordlen;
set of int: widx = 1..wordlen;

var int: x;
var bool: y;

array[1..1] of 1..96: EMPTY=[96];
int: len_EMPTY = 0;

var 0..wordlen: len_M7_13; array[widx] of var symbols: M7_13;
constraint str_pad(M7_13, len_M7_13);

constraint str_equal(M7_13, len_M7_13, EMPTY, len_EMPTY);

constraint x == len_M7_13;

constraint y <-> 0x0 <= x;

constraint y == true;
solve satisfy;

output ["x := ", show(x), "\n"];

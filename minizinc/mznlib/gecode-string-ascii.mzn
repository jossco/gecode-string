%% gecode-string:	bounded-length string constraints for Gecode
%% url: 		github.com/jossco/gecode-string
%% author: 	Joseph D. Scott, Uppsala University
%% ========
%% file: 		./minizinc/mznlib/gecode-string-ascii.mzn
%% version: 	0.2.1
%% date: 		Wed Dec  3 17:58:46 CET 2014
%% ========

int: max_symbol = 96;
set of int: symbols = 1..max_symbol;
% offset by removing first 31 characters of ascii-127
array[1..max_symbol] of string: ascii = 
[ "»", "!", "\"", "#", "$", "%", "&", "'", "(", ")", 
  "*", "+", ",", "-", ".", "/", "0", "1", "2", "3", 
  "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", 
  ">", "?", "@", "A", "B", "C", "D", "E", "F", "G", 
  "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", 
  "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", 
  " ", "]", "^", "_", "`", "a", "b", "c", "d", "e", 
  "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", 
  "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", 
  "z", "{", "|", "}", "~", "≠"
];


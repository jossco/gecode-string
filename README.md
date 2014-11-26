Author: Joseph Scott, Uppsala University
        
This is a prototype implementation. Use at your own risk.

This is an implementation of an Gecode extension to handle bounded-length strings.
At the moment, a string is represented by two components:
* an array of symbol variables (IntVar only at the moment)
* an IntVar length
    
Constraints include equal, concat, substring, character_at, and an implementation of Michael Maher's bounded open version of the regular constraint.

There is also a  [technical report](https://github.com/jossco/gecode-string/releases/download/v0.2/abstract-domain.pdf) outlining the theoretical foundation for the bounded-length string domain

## Build instructions

"make all" should do the trick (but see below).

## Dependencies
* www.gecode.org — Gecode (4.2 or greater)
* [openfst.org](http://openfst.org/twiki/bin/view/FST/WebHome) — openFST (tested with 1.4.1)

## Makefile
The makefile is not completely automated. Here are a few things you may need to adjust:

1. GECODE needs to point to your gecode install directory
    (If you've built but not installed gecode, you need to override the GECODE_LIB and GECODE_INCL variables appropriately.)
2. Architecture is determined by the ARCH variable:
  * "linux")  (DEFAULT) 
    * You probably need to set LD_LIBRARY_PATH to include both the Gecode and openFST libraries 
    * (usually LD_LIBRARY_PATH=/usr/local/lib).
  * "mac")  
    * You'll either have to install g++-4.8, or fix the Makefile to work with clang
    * And set DYLD_LIBRARY_PATH
  * "windows")
    *erm...sorry, I have no idea. Lycka till!
3. To use Gecode with gist, set GIST=true
4. The minizinc executables (i.e., [mf]zn-gecode-string) require that you have built gecode from source:
  1. Set GECODE_SRC to point to the root directory of the gecode source
  2. You may need to change the library files included when linking fzn-gecode-string, in order to accurately reflect your gecode build. If you built gecode with float variables, for example, then fzn-gecode-string must be linked to -libgecodefloat.

Author: Joseph Scott
        Uppsala University
        
This is a prototype implementation. Use at your own risk.

This is an implementation of an Gecode extension to handle bounded-length strings.
At the moment, a string is represented by two components:
    - an array of symbol variables (IntVar only at the moment)
    - an IntVar length
    
Constraints include equal, concat, substring, character_at, and an implementation of Michael Maher's bounded open version of the regular constraint.

Build instructions:

"make all" should do the trick.

Dependencies:
    www.gecode.org  Gecode-4.2.1 (anything 4.x.x should work)
    openfst.org     openFST-1.3.4


The makefile is not completely automated. Here are a few things you may need to adjust:

1) If you're gecode is somewhere odd, either set the GECODE_BASE variable, or edit the path directly.
2) If you are building on Linux:
     a) You probably need to set LD_LIBRARY_PATH to include both the Gecode and openFST libraries (usually LD_LIBRARY_PATH=/usr/local/lib).
     b) You DEFINITELY need to uncomment the line "LDLIBS += -ldl"
3) If you don't have gist enabled on your Gecode install, set GIST=false
4) If you are building on Windows... erm, sorry, I have no idea. Lycka till!
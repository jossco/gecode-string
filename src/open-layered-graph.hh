namespace Gecode {
	/**
	* \brief Post open domain consistent propagator for extensional constraint described by a DFA
	*
	* The first \a n elements of \a x must be a word of the language described by
	* the DFA \a d.
	*
	* Throws an exception of type Int::ArgumentSame, if \a x contains
	* the same unassigned variable multiply. If shared occurences of variables
	* are required, unshare should be used.
	*/
	GECODE_INT_EXPORT void
		extensional(Home home, const IntVarArgs& x, DFA d, const IntVar n);
}
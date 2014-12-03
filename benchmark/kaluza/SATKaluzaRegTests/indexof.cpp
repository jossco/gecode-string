/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./benchmark/kaluza/SATKaluzaRegTests/indexof.cpp
version: 	0.2.1
date: 		Wed Dec  3 17:58:46 CET 2014
========
Constraint model by Jun He
*/

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>

#include <gecode/driver.hh>
#include <gecode/minimodel.hh>

#include "open.hh"
#include "open-layered-graph.hh"

#define CSTRING(X,X_n,cstr) \
	str = cstr; \
	X = IntVarArray(*this, wordlength); \
	for (unsigned int i = 0; i < str.length(); i++) \
		X[i] = IntVar(*this, (int)str[i] - 31, (int)str[i] - 31); \
	for (int i=(int)str.length(); i<X.size(); i++) \
			X[i] = IntVar(*this, dummy_sym, dummy_sym); \
	X_n = IntVar(*this, str.length(), str.length());

#define PAD_INVARIANT(home, X, Xn) \
	for(int i=0; i<wordlength; i++) \
		rel(home,(X[i]==dummy_sym) == (Xn<=i));

#define PAD_EQUAL(home, X,Xn,Y,Yn) \
	rel(home, Xn == Yn); \
	for (int i = 0; i<wordlength;i++) \
		rel(home, (i<Xn) >> (X[i]==Y[i])); 
	
using namespace Gecode;
using namespace std;

int wordlength = 0;
int dummy_sym = Open::OpenString::padchar;
int val_dom_min = -1;
int val_dom_max = 96;

std::ostream&
select_ostream(const char* name, std::ofstream& ofs) {
  if (strcmp(name, "stdout") == 0) {
    return std::cout;
  } else if (strcmp(name, "stdlog") == 0) {
    return std::clog;
  } else if (strcmp(name, "stderr") == 0) {
    return std::cerr;
  } else {
    ofs.open(name);
    return ofs;
  }
}

class KaluzaExample : public Script {
	IntVar PCTEMP_LHS_1, var_0xINPUT_2_n, T0_1_n, T1_1_n, T2_1_n, T3_1_n, T4_1_n, T5_1_n;
	IntVarArray var_0xINPUT_2, T0_1, T1_1, T2_1, T3_1, T4_1, T5_1;
	//protected:
	// void constantString(IntVarArray X, IntVar Xn, const char* str) {
	// 	int len = strlen(str);
	// 	Xn = IntVar(*this, len, len);
	// 	X = IntVarArray(*this, len + 1);
	// 	for (int i = 0; i < len; i++) {
	// 		int modascii = (int)str[i] - 31;
	// 		X[i] = IntVar(*this, modascii, modascii);
	// 	}
	// 	X[len] = IntVar(*this, dummy_sym, dummy_sym);
	// }
public:
	enum {BRANCH_A_N, BRANCH_N_A, SEARCH_DFS, PROP_OPEN, PROP_PAD};
	KaluzaExample(const SizeOptions& opt)
		: PCTEMP_LHS_1(*this,0,wordlength),
			var_0xINPUT_2_n(*this,0,wordlength),
	    T0_1_n(*this,0,wordlength),
			T1_1_n(*this,0,wordlength),
			T2_1_n(*this,0,wordlength),
			T3_1_n(*this,0,wordlength),
			T4_1_n(*this,0,wordlength),
			T5_1_n(*this,0,wordlength),
			var_0xINPUT_2(*this,wordlength,val_dom_min,val_dom_max),
			T0_1(*this,wordlength,val_dom_min,val_dom_max),
			T1_1(*this,wordlength,val_dom_min,val_dom_max),
			T2_1(*this,wordlength,val_dom_min,val_dom_max),
			T3_1(*this,wordlength,val_dom_min,val_dom_max),
			T4_1(*this,wordlength,val_dom_min,val_dom_max),
			T5_1(*this,wordlength,val_dom_min,val_dom_max){

		// constant strings
		string str;
		CSTRING(T5_1, T5_1_n, "=");
		
		// T4_1 \notin CapturedBrack(/=/, 0);
		// take advantage of the fact that str = "="
		for(int i=0; i<T4_1.size(); i++)
			rel(*this, T4_1[i] != (int)str[0] - 31);
		
		// length constraints:
		rel(*this, PCTEMP_LHS_1==5);
		//rel(*this, PCTEMP_LHS_1==I0_1);
		rel(*this, T0_1_n==0);
		rel(*this, T4_1_n==PCTEMP_LHS_1);
		
		switch(opt.propagation()){
			case PROP_PAD:
			PAD_INVARIANT(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			PAD_INVARIANT(*this,T0_1, T0_1_n);
			PAD_INVARIANT(*this,T1_1, T1_1_n);
			PAD_INVARIANT(*this,T2_1, T2_1_n);
			PAD_INVARIANT(*this,T3_1, T3_1_n);
			PAD_INVARIANT(*this,T4_1, T4_1_n);
			PAD_INVARIANT(*this,T5_1, T5_1_n);
			
			//concat(T0_1, T0_1_n, T1_1, T1_1_n, var_0xINPUT_2, var_0xINPUT_2_n);
			rel(*this, var_0xINPUT_2_n == (T0_1_n+T1_1_n)); // Zn=Xn+Yn
			for(int i=0; i<wordlength; i++) 
			{
				rel(*this, (i<T0_1_n) >> (T0_1[i]==var_0xINPUT_2[i]));	// i<Xn --> X[i]=Z[i]
				BoolVar b = expr(*this, i==T0_1_n);						// b    <-> i=Xn
				for(int j=0; (i+j)<wordlength; j++)	//(i+j)<|Z|&j<|Y|
					rel(*this, (b&&(j<T1_1_n)) >> (T1_1[j]==var_0xINPUT_2[i+j]));	// b&(j<Yn) --> Y[i]=Z[i]
			}
			//concat(*this, T2_1, T2_1_n, T3_1, T3_1_n, T1_1, T1_1_n);
			rel(*this, T1_1_n == (T2_1_n+T3_1_n)); // Zn=Xn+Yn
			for(int i=0; i<wordlength; i++) 
			{
				rel(*this, (i<T2_1_n) >> (T2_1[i]==T1_1[i]));	// i<Xn --> X[i]=Z[i]
				BoolVar b = expr(*this, i==T2_1_n);						// b    <-> i=Xn
				for(int j=0; (i+j)<wordlength; j++)	//(i+j)<|Z|&j<|Y|
					rel(*this, (b&&(j<T3_1_n)) >> (T3_1[j]==T1_1[i+j]));	// b&(j<Yn) --> Y[i]=Z[i]
			}
			//concat(*this, T4_1, T4_1_n, T5_1, T5_1_n, T2_1, T2_1_n);
			rel(*this, T2_1_n == (T4_1_n+T5_1_n)); // Zn=Xn+Yn
			for(int i=0; i<wordlength; i++) 
			{
				rel(*this, (i<T4_1_n) >> (T4_1[i]==T2_1[i]));	// i<Xn --> X[i]=Z[i]
				BoolVar b = expr(*this, i==T4_1_n);						// b    <-> i=Xn
				for(int j=0; (i+j)<wordlength; j++)	//(i+j)<|Z|&j<|Y|
					rel(*this, (b&&(j<T5_1_n)) >> (T5_1[j]==T2_1[i+j]));	// b&(j<Yn) --> Y[i]=Z[i]
			}
			break;
			
			case PROP_OPEN:
			
			open_invariant(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			open_invariant(*this,T0_1, T0_1_n);
			open_invariant(*this,T1_1, T1_1_n);
			open_invariant(*this,T2_1, T2_1_n);
			open_invariant(*this,T3_1, T3_1_n);
			open_invariant(*this,T4_1, T4_1_n);
			open_invariant(*this,T5_1, T5_1_n);

			
			open_concat(*this, T0_1, T0_1_n, T1_1, T1_1_n, var_0xINPUT_2, var_0xINPUT_2_n);
			open_concat(*this, T2_1, T2_1_n, T3_1, T3_1_n, T1_1, T1_1_n);
			open_concat(*this, T4_1, T4_1_n, T5_1, T5_1_n, T2_1, T2_1_n);
			break;
		}
		branch(*this, var_0xINPUT_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T0_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T2_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T3_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T4_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T5_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}
	
	KaluzaExample(bool share, KaluzaExample& s)
	: Script(share,s) {
			PCTEMP_LHS_1.update(*this, share, s.PCTEMP_LHS_1);
			var_0xINPUT_2_n.update(*this, share, s.var_0xINPUT_2_n);
			T0_1_n.update(*this, share, s.T0_1_n);
			T1_1_n.update(*this, share, s.T1_1_n);
			T2_1_n.update(*this, share, s.T2_1_n);
			T3_1_n.update(*this, share, s.T3_1_n);
			T4_1_n.update(*this, share, s.T4_1_n);
			T5_1_n.update(*this, share, s.T5_1_n);
			var_0xINPUT_2.update(*this, share, s.var_0xINPUT_2);
			T0_1.update(*this, share, s.T0_1);
			T1_1.update(*this, share, s.T1_1);
			T2_1.update(*this, share, s.T2_1);
			T3_1.update(*this, share, s.T3_1);
			T4_1.update(*this, share, s.T4_1);
			T5_1.update(*this, share, s.T5_1);

	}

	virtual Space* copy(bool share) {
		return new KaluzaExample(share,*this);
	}

	virtual void print(std::ostream& os) const {
		os << "bound = " << wordlength << endl; 
		os << "var_0xINPUT_2 = ";
		Open::OpenString::print(os,var_0xINPUT_2,var_0xINPUT_2_n);
		os << std::endl;
		Open::OpenString::printAsString(os,var_0xINPUT_2,var_0xINPUT_2_n);
	}
};

int main(int argc, char* argv[]) {
	SizeOptions opt("Kaluza Example");
	opt.solutions(1); // only need one solution
  opt.size(30);
  opt.propagation(KaluzaExample::PROP_OPEN,   "open",    "bounded-length extensional propagation");
  opt.propagation(KaluzaExample::PROP_PAD,    "pad",     "fixed, maximal length string with padding characters");
  opt.propagation(KaluzaExample::PROP_OPEN);
	opt.parse(argc,argv);

  wordlength = opt.size();
	cout << "length = " << wordlength << endl;
  Script::run<KaluzaExample,DFS,SizeOptions>(opt);
	
	return 0;
}




/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./benchmark/kaluza/SATKaluzaRegTests/replace.cpp
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
	IntVar var_0xINPUT_2_n, T0_2_n, T1_2_n, T2_2_n, T3_2_n, T4_2_n, T5_2_n, PCTEMP_LHS_1_n, T0_4_n, T1_4_n, T2_4_n, T3_4_n, T4_4_n, T5_4_n, PCTEMP_LHS_2;
	IntVarArray var_0xINPUT_2, T0_2, T1_2, T2_2, T3_2, T4_2, T5_2, PCTEMP_LHS_1, T0_4, T1_4, T2_4, T3_4, T4_4, T5_4;
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
		: var_0xINPUT_2_n(*this,0,wordlength),
			T0_2_n(*this,0,wordlength),
			T1_2_n(*this,0,wordlength),
			T3_2_n(*this,0,wordlength),
			T4_2_n(*this,0,wordlength),
			PCTEMP_LHS_1_n(*this,0,wordlength),
			T0_4_n(*this,0,wordlength),
			T1_4_n(*this,0,wordlength),
			T2_4_n(*this,0,wordlength),
			T3_4_n(*this,0,wordlength),
			T4_4_n(*this,0,wordlength),
			PCTEMP_LHS_2(*this,0,wordlength),
			var_0xINPUT_2(*this,wordlength,val_dom_min,val_dom_max),
			T0_2(*this,wordlength,val_dom_min,val_dom_max),
			T1_2(*this,wordlength,val_dom_min,val_dom_max),
			T3_2(*this,wordlength,val_dom_min,val_dom_max),
			T4_2(*this,wordlength,val_dom_min,val_dom_max),
			PCTEMP_LHS_1(*this,wordlength,val_dom_min,val_dom_max),
			T0_4(*this,wordlength,val_dom_min,val_dom_max),
			T1_4(*this,wordlength,val_dom_min,val_dom_max),
			T2_4(*this,wordlength,val_dom_min,val_dom_max),
			T3_4(*this,wordlength,val_dom_min,val_dom_max),
			T4_4(*this,wordlength,val_dom_min,val_dom_max){
		// constant strings
		string str;
		CSTRING(T5_2,T5_2_n,"_");
		CSTRING(T2_2,T2_2_n,"=");
		CSTRING(T5_4,T5_4_n,"_");
		//T0_2 \notin CapturedBrack(/=/, 0);
		str = "=";
		for(int i=0; i<wordlength; i++)
			rel(*this,T0_2[i]!=(int)str[0]);
		//T4_4 \notin CapturedBrack(/_/, 0);
		str="_";
		for(int i=0; i<wordlength; i++)
			rel(*this,T4_4[i]!=(int)str[0]);
		//PCTEMP_LHS_2 == 0x5;
		rel(*this, PCTEMP_LHS_2 == 5);
		//PCTEMP_LHS_2 := I0_4 + 0x0;
		//I0_4 == Len(T4_4);
		rel(*this, T4_4_n == PCTEMP_LHS_2);
		//0x0 == Len(T0_4);
		rel(*this, T0_4_n == 0);
	
		switch(opt.propagation()){
			case PROP_PAD:
			{
				PAD_INVARIANT(*this,var_0xINPUT_2, var_0xINPUT_2_n);
				PAD_INVARIANT(*this,T0_2, T0_2_n);
				PAD_INVARIANT(*this,T1_2, T1_2_n);
				PAD_INVARIANT(*this,T2_2, T2_2_n);
				PAD_INVARIANT(*this,T3_2, T3_2_n);
				PAD_INVARIANT(*this,T4_2, T4_2_n);
				PAD_INVARIANT(*this,T5_2, T5_2_n);
				PAD_INVARIANT(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
				PAD_INVARIANT(*this,T0_4, T0_4_n);
				PAD_INVARIANT(*this,T1_4, T1_4_n);
				PAD_INVARIANT(*this,T2_4, T2_4_n);
				PAD_INVARIANT(*this,T3_4, T3_4_n);
				PAD_INVARIANT(*this,T4_4, T4_4_n);
				PAD_INVARIANT(*this,T5_4, T5_4_n);
			
				//var_0xINPUT_2 := T0_2 . T1_2;
				rel(*this, var_0xINPUT_2_n == (T0_2_n+T1_2_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T0_2_n) >> (T0_2[i]==var_0xINPUT_2[i]));	
					BoolVar b = expr(*this, i==T0_2_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T1_2_n)) >> (T1_2[j]==var_0xINPUT_2[i+j]));	
				}
				//T1_2 := T2_2 . T3_2
				rel(*this, T1_2_n == (T2_2_n+T3_2_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T2_2_n) >> (T2_2[i]==T1_2[i]));	
					BoolVar b = expr(*this, i==T2_2_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T3_2_n)) >> (T3_2[j]==T1_2[i+j]));	
				}
				//T4_2 := T5_2 . T3_2;
				rel(*this, T4_2_n == (T5_2_n+T3_2_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T5_2_n) >> (T5_2[i]==T4_2[i]));	
					BoolVar b = expr(*this, i==T5_2_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T3_2_n)) >> (T3_2[j]==T4_2[i+j]));	
				}
				//PCTEMP_LHS_1 := T0_2 . T4_2;
				rel(*this, PCTEMP_LHS_1_n == (T0_2_n+T4_2_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T0_2_n) >> (T0_2[i]==PCTEMP_LHS_1[i]));	
					BoolVar b = expr(*this, i==T0_2_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T4_2_n)) >> (T4_2[j]==PCTEMP_LHS_1[i+j]));	
				}
				//PCTEMP_LHS_1 := T0_4 . T1_4;
				rel(*this, PCTEMP_LHS_1_n == (T0_4_n+T1_4_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T0_4_n) >> (T0_4[i]==PCTEMP_LHS_1[i]));	
					BoolVar b = expr(*this, i==T0_4_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T1_4_n)) >> (T1_4[j]==PCTEMP_LHS_1[i+j]));	
				}
				//T1_4 := T2_4 . T3_4;
				rel(*this, T1_4_n == (T2_4_n+T3_4_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T2_4_n) >> (T2_4[i]==T1_4[i]));	
					BoolVar b = expr(*this, i==T2_4_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T3_4_n)) >> (T3_4[j]==T1_4[i+j]));	
				}
				//T2_4 := T4_4 . T5_4;
				rel(*this, T2_4_n == (T4_4_n+T5_4_n)); 
				for(int i=0; i<wordlength; i++) {
					rel(*this, (i<T4_4_n) >> (T4_4[i]==T2_4[i]));	
					BoolVar b = expr(*this, i==T4_4_n);						
					for(int j=0; (i+j)<wordlength; j++)
						rel(*this, (b&&(j<T5_4_n)) >> (T5_4[j]==T2_4[i+j]));	
				}
				break;}
				case PROP_OPEN:
				{
					open_invariant(*this,var_0xINPUT_2, var_0xINPUT_2_n);
					open_invariant(*this,T0_2, T0_2_n);
					open_invariant(*this,T1_2, T1_2_n);
					open_invariant(*this,T2_2, T2_2_n);
					open_invariant(*this,T3_2, T3_2_n);
					open_invariant(*this,T4_2, T4_2_n);
					open_invariant(*this,T5_2, T5_2_n);
					open_invariant(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
					open_invariant(*this,T0_4, T0_4_n);
					open_invariant(*this,T1_4, T1_4_n);
					open_invariant(*this,T2_4, T2_4_n);
					open_invariant(*this,T3_4, T3_4_n);
					open_invariant(*this,T4_4, T4_4_n);
					open_invariant(*this,T5_4, T5_4_n);

					//var_0xINPUT_2 := T0_2 . T1_2;
					open_concat(*this, T0_2, T0_2_n, T1_2, T1_2_n, var_0xINPUT_2, var_0xINPUT_2_n);
					//T1_2 := T2_2 . T3_2
					open_concat(*this, T2_2, T2_2_n, T3_2, T3_2_n, T1_2, T1_2_n);
					//T4_2 := T5_2 . T3_2;
					open_concat(*this, T5_2, T5_2_n, T3_2, T3_2_n, T4_2, T4_2_n);
					//PCTEMP_LHS_1 := T0_2 . T4_2;
					open_concat(*this, T0_2, T0_2_n, T4_2, T4_2_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
					//PCTEMP_LHS_1 := T0_4 . T1_4;
					open_concat(*this, T0_4, T0_4_n, T1_4, T1_4_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
					//T1_4 := T2_4 . T3_4;
					open_concat(*this, T2_4, T2_4_n, T3_4, T3_4_n, T1_4, T1_4_n);
					//T2_4 := T4_4 . T5_4;
					open_concat(*this, T4_4, T4_4_n, T5_4, T5_4_n, T2_4, T2_4_n);
					break;
				}
			}
		branch(*this, var_0xINPUT_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T0_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T2_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T3_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T4_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T5_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, PCTEMP_LHS_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T0_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T2_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T3_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T4_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T5_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}
	
	KaluzaExample(bool share, KaluzaExample& s)
	: Script(share,s) {
					var_0xINPUT_2.update(*this, share, s.var_0xINPUT_2);
			T0_2.update(*this, share, s.T0_2);
			T1_2.update(*this, share, s.T1_2);
			T2_2.update(*this, share, s.T2_2);
			T3_2.update(*this, share, s.T3_2);
			T4_2.update(*this, share, s.T4_2);
			T5_2.update(*this, share, s.T5_2);
			PCTEMP_LHS_1.update(*this, share, s.PCTEMP_LHS_1);
			T0_4.update(*this, share, s.T0_4);
			T1_4.update(*this, share, s.T1_4);
			T2_4.update(*this, share, s.T2_4);
			T3_4.update(*this, share, s.T3_4);
			T4_4.update(*this, share, s.T4_4);
			T5_4.update(*this, share, s.T5_4);
			var_0xINPUT_2_n.update(*this, share, s.var_0xINPUT_2_n);
			T0_2_n.update(*this, share, s.T0_2_n);
			T1_2_n.update(*this, share, s.T1_2_n);
			T2_2_n.update(*this, share, s.T2_2_n);
			T3_2_n.update(*this, share, s.T3_2_n);
			T4_2_n.update(*this, share, s.T4_2_n);
			T5_2_n.update(*this, share, s.T5_2_n);
			PCTEMP_LHS_1_n.update(*this, share, s.PCTEMP_LHS_1_n);
			T0_4_n.update(*this, share, s.T0_4_n);
			T1_4_n.update(*this, share, s.T1_4_n);
			T2_4_n.update(*this, share, s.T2_4_n);
			T3_4_n.update(*this, share, s.T3_4_n);
			T4_4_n.update(*this, share, s.T4_4_n);
			T5_4_n.update(*this, share, s.T5_4_n);
			PCTEMP_LHS_2.update(*this, share, s.PCTEMP_LHS_2);
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




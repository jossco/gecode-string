/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./benchmark/kaluza/SATKaluzaRegTests/streq.cpp
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
	IntVar I0_2, I1_2, I2_2, PCTEMP_LHS_1_n, var_0xINPUT_19_n, T1_2_n, T2_2_n, T3_2_n;
	IntVarArray PCTEMP_LHS_1, var_0xINPUT_19, T1_2, T2_2, T3_2;
public:
	enum {BRANCH_A_N, BRANCH_N_A, SEARCH_DFS, PROP_OPEN, PROP_PAD};
	KaluzaExample(const SizeOptions& opt)
		: I0_2(*this,0,wordlength),
			I1_2(*this,0,wordlength),
			I2_2(*this,0,wordlength),
			PCTEMP_LHS_1_n(*this,0,wordlength),
			var_0xINPUT_19_n(*this,0,wordlength),
			T1_2_n(*this,0,wordlength),
			T2_2_n(*this,0,wordlength),
			T3_2_n(*this,0,wordlength),
			PCTEMP_LHS_1(*this,wordlength,val_dom_min,val_dom_max),
			var_0xINPUT_19(*this,wordlength,val_dom_min,val_dom_max),
			T1_2(*this,wordlength,val_dom_min,val_dom_max),
			T2_2(*this,wordlength,val_dom_min,val_dom_max),
			T3_2(*this,wordlength,val_dom_min,val_dom_max){
		// constant strings
		string str;
		//PCTEMP_LHS_1 == "Hello";
		CSTRING(PCTEMP_LHS_1, PCTEMP_LHS_1_n, "Hello");
		rel(*this, I0_2==5);
		rel(*this, I1_2>=5);
		rel(*this, I2_2==I1_2);
		rel(*this, I0_2==PCTEMP_LHS_1_n);
		rel(*this, T1_2_n==0);
		rel(*this, I1_2==var_0xINPUT_19_n);
		switch(opt.propagation()){
			case PROP_PAD:
			PAD_INVARIANT(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			PAD_INVARIANT(*this,var_0xINPUT_19, var_0xINPUT_19_n);
			PAD_INVARIANT(*this,T1_2, T1_2_n);
			PAD_INVARIANT(*this,T2_2, T2_2_n);
			PAD_INVARIANT(*this,T3_2, T3_2_n);
			
			//var_0xINPUT_19 := T1_2 . T2_2;
			rel(*this, var_0xINPUT_19_n == (T1_2_n+T2_2_n)); 
			for(int i=0; i<wordlength; i++) {
				rel(*this, (i<T1_2_n) >> (T1_2[i]==var_0xINPUT_19[i]));	
				BoolVar b = expr(*this, i==T1_2_n);						
				for(int j=0; (i+j)<wordlength; j++)
					rel(*this, (b&&(j<T2_2_n)) >> (T2_2[j]==var_0xINPUT_19[i+j]));	
			}
			//T2_2 := PCTEMP_LHS_1 . T3_2;
			rel(*this, T2_2_n == (PCTEMP_LHS_1_n+T3_2_n)); 
			for(int i=0; i<wordlength; i++) {
				rel(*this, (i<PCTEMP_LHS_1_n) >> (PCTEMP_LHS_1[i]==T2_2[i]));	
				BoolVar b = expr(*this, i==PCTEMP_LHS_1_n);						
				for(int j=0; (i+j)<wordlength; j++)
					rel(*this, (b&&(j<T3_2_n)) >> (T3_2[j]==T2_2[i+j]));	
			}
			break;
			case PROP_OPEN:
			
			open_invariant(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			open_invariant(*this,var_0xINPUT_19, var_0xINPUT_19_n);
			open_invariant(*this,T1_2, T1_2_n);
			open_invariant(*this,T2_2, T2_2_n);
			open_invariant(*this,T3_2, T3_2_n);

			//var_0xINPUT_19 := T1_2 . T2_2;
			//T2_2 := PCTEMP_LHS_1 . T3_2;
			open_concat(*this, T1_2, T1_2_n, T2_2, T2_2_n, var_0xINPUT_19, var_0xINPUT_19_n);
			open_concat(*this, PCTEMP_LHS_1, PCTEMP_LHS_1_n, T3_2, T3_2_n, T2_2, T2_2_n);
			break;
		}
		
		branch(*this, PCTEMP_LHS_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, var_0xINPUT_19, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T2_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T3_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());

	}
	
	KaluzaExample(bool share, KaluzaExample& s)
	: Script(share,s) {
					PCTEMP_LHS_1.update(*this, share, s.PCTEMP_LHS_1);
			var_0xINPUT_19.update(*this, share, s.var_0xINPUT_19);
			T1_2.update(*this, share, s.T1_2);
			T2_2.update(*this, share, s.T2_2);
			T3_2.update(*this, share, s.T3_2);
			I0_2.update(*this, share, s.I0_2);
			I1_2.update(*this, share, s.I1_2);
			I2_2.update(*this, share, s.I2_2);
			PCTEMP_LHS_1_n.update(*this, share, s.PCTEMP_LHS_1_n);
			var_0xINPUT_19_n.update(*this, share, s.var_0xINPUT_19_n);
			T1_2_n.update(*this, share, s.T1_2_n);
			T2_2_n.update(*this, share, s.T2_2_n);
			T3_2_n.update(*this, share, s.T3_2_n);

	}

	virtual Space* copy(bool share) {
		return new KaluzaExample(share,*this);
	}

	virtual void print(std::ostream& os) const {
		os << "bound = " << wordlength << endl; 
		os << "var_0xINPUT_19 = ";
		Open::OpenString::print(os,var_0xINPUT_19,var_0xINPUT_19_n);
		os << std::endl;
		Open::OpenString::printAsString(os,var_0xINPUT_19,var_0xINPUT_19_n);
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




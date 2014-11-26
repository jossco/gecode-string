/*
Author:   Joseph D. Scott
          Uppsala University

Used in:
          "Constraint Solving on Bounded String Variables"
          Scott, J.D., Flener, P. and Pearson, J.
          submitted to CPAIOR 2015
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
	IntVar PCTEMP_LHS_1_n, T1_1_n, var_0xINPUT_2_n, T1_4_n, T2_1_n, T2_4_n, T_2_n;
	IntVarArray PCTEMP_LHS_1, T1_1, var_0xINPUT_2, T1_4, T2_1, T2_4, T_2;
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
		: PCTEMP_LHS_1_n(*this, 1, wordlength), 
	T1_1_n(*this, 1, wordlength), 
	var_0xINPUT_2_n(*this, 1, wordlength),
	T1_4_n(*this, 1, wordlength),
	PCTEMP_LHS_1(*this, wordlength , val_dom_min, val_dom_max),
	T1_1(*this, wordlength , val_dom_min, val_dom_max),
	var_0xINPUT_2(*this, wordlength , val_dom_min, val_dom_max),
	T1_4(*this, wordlength , val_dom_min, val_dom_max){

		// constant strings
		string str;
		CSTRING(T2_1, T2_1_n, "=Online");
		CSTRING(T2_4, T2_4_n, "Now");
		CSTRING(T_2, T_2_n, "Hello=Joe=OnlineNow");

		switch(opt.propagation()){
			case PROP_PAD:
			PAD_INVARIANT(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			PAD_INVARIANT(*this,T1_1, T1_1_n);
			PAD_INVARIANT(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			PAD_INVARIANT(*this,T1_4, T1_4_n);
			
			//concat(*this, T1_1, T1_1_n, T2_1, T2_1_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			rel(*this, PCTEMP_LHS_1_n == (T1_1_n+T2_1_n));
			for(int i=0; i<wordlength; i++)
			{
				rel(*this, (i<T1_1_n) >> (T1_1[i]==PCTEMP_LHS_1[i]));
				BoolVar b = expr(*this, i==T1_1_n);
				for(int j=0; (i+j)<wordlength; j++)
					rel(*this, (b&&(j<T2_1_n)) >> (T2_1[j]==PCTEMP_LHS_1[i+j]));
			}
			//concat(*this, T1_4, T1_4_n, T2_4, T2_4_n, T_2, T_2_n);
			rel(*this, T_2_n == (T1_4_n+T2_4_n)); // Xn+Yn=Zn
			for(int i=0; i<wordlength; i++) // i<X.size()
			{
				rel(*this, (i<T1_4_n) >> (T1_4[i]==T_2[i]));	// i<Xn --> X[i]=Z[i]
				BoolVar b = expr(*this, i==T1_4_n);						// b    <-> i=Xn
				for(int j=0; (i+j)<wordlength; j++)	//(i+j)<|Z|&j<|Y|
					rel(*this, (b&&(j<T2_4_n)) >> (T2_4[j]==T_2[i+j]));	// b&(j<Yn) --> Y[i]=Z[i]
			}
			PAD_EQUAL(*this, T1_1, T1_1_n, var_0xINPUT_2, var_0xINPUT_2_n);
			PAD_EQUAL(*this, T1_4, T1_4_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			break;
			case PROP_OPEN:
			
			open_invariant(*this,PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			open_invariant(*this,T1_1, T1_1_n);
			open_invariant(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			open_invariant(*this,T1_4, T1_4_n);
			
			open_concat(*this, T1_1, T1_1_n, T2_1, T2_1_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			open_concat(*this, T1_4, T1_4_n, T2_4, T2_4_n, T_2, T_2_n);
			open_equal(*this, T1_1, T1_1_n, var_0xINPUT_2, var_0xINPUT_2_n);
			open_equal(*this, T1_4, T1_4_n, PCTEMP_LHS_1, PCTEMP_LHS_1_n);
			break;
		}
		
		branch(*this, PCTEMP_LHS_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_1, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, var_0xINPUT_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_4, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
	
	KaluzaExample(bool share, KaluzaExample& s)
	: Script(share,s) {
		PCTEMP_LHS_1.update(*this, share, s.PCTEMP_LHS_1);
		T1_1.update(*this, share, s.T1_1);
		var_0xINPUT_2.update(*this, share, s.var_0xINPUT_2);
		T1_4.update(*this, share, s.T1_4);
		T2_1.update(*this, share, s.T2_1);
		T2_4.update(*this, share, s.T2_4);
		T_2.update(*this, share, s.T_2);

		PCTEMP_LHS_1_n.update(*this, share, s.PCTEMP_LHS_1_n);
		T1_1_n.update(*this, share, s.T1_1_n);
		var_0xINPUT_2_n.update(*this, share, s.var_0xINPUT_2_n);
		T1_4_n.update(*this, share, s.T1_4_n);
		T2_1_n.update(*this, share, s.T2_1_n);
		T2_4_n.update(*this, share, s.T2_4_n);
		T_2_n.update(*this, share, s.T_2_n);
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




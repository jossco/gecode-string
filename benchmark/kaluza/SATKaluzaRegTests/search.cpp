/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./benchmark/kaluza/SATKaluzaRegTests/search.cpp
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

Gecode::DFA::Transition bigp[] = {
	{0,1,0},
	{0,2,0},
	{0,3,1},
	{0,4,0},
	{0,5,0},
	{0,6,0},
	{0,7,0},
	{0,8,0},
	{0,9,0},
	{0,10,0},
	{0,11,0},
	{0,12,0},
	{0,13,0},
	{0,14,0},
	{0,15,0},
	{0,16,0},
	{0,17,0},
	{0,18,0},
	{0,19,0},
	{0,20,0},
	{0,21,0},
	{0,22,0},
	{0,23,0},
	{0,24,0},
	{0,25,0},
	{0,26,0},
	{0,27,0},
	{0,28,0},
	{0,29,0},
	{0,30,0},
	{0,31,0},
	{0,32,0},
	{0,33,0},
	{0,34,0},
	{0,35,0},
	{0,36,0},
	{0,37,0},
	{0,38,0},
	{0,39,0},
	{0,40,0},
	{0,41,0},
	{0,42,0},
	{0,43,0},
	{0,44,0},
	{0,45,0},
	{0,46,0},
	{0,47,0},
	{0,48,0},
	{0,49,0},
	{0,50,0},
	{0,51,0},
	{0,52,0},
	{0,53,0},
	{0,54,0},
	{0,55,0},
	{0,56,0},
	{0,57,0},
	{0,58,0},
	{0,59,0},
	{0,60,0},
	{0,61,0},
	{0,62,0},
	{0,63,0},
	{0,64,0},
	{0,65,0},
	{0,66,0},
	{0,67,0},
	{0,68,0},
	{0,69,0},
	{0,70,0},
	{0,71,0},
	{0,72,0},
	{0,73,0},
	{0,74,0},
	{0,75,0},
	{0,76,0},
	{0,77,0},
	{0,78,0},
	{0,79,0},
	{0,80,0},
	{0,81,0},
	{0,82,0},
	{0,83,0},
	{0,84,0},
	{0,85,0},
	{0,86,0},
	{0,87,0},
	{0,88,0},
	{0,89,0},
	{0,90,0},
	{0,91,0},
	{0,92,0},
	{0,93,0},
	{0,94,0},
	{0,95,0},
	{0,96,0},
	{1,1,0},
	{1,2,0},
	{1,3,0},
	{1,4,0},
	{1,5,0},
	{1,6,0},
	{1,7,0},
	{1,8,2},
	{1,9,0},
	{1,10,0},
	{1,11,0},
	{1,12,0},
	{1,13,0},
	{1,14,0},
	{1,15,0},
	{1,16,0},
	{1,17,0},
	{1,18,0},
	{1,19,0},
	{1,20,0},
	{1,21,0},
	{1,22,0},
	{1,23,0},
	{1,24,0},
	{1,25,0},
	{1,26,0},
	{1,27,0},
	{1,28,0},
	{1,29,0},
	{1,30,0},
	{1,31,0},
	{1,32,0},
	{1,33,0},
	{1,34,0},
	{1,35,0},
	{1,36,0},
	{1,37,0},
	{1,38,0},
	{1,39,0},
	{1,40,0},
	{1,41,0},
	{1,42,0},
	{1,43,0},
	{1,44,0},
	{1,45,0},
	{1,46,0},
	{1,47,0},
	{1,48,0},
	{1,49,0},
	{1,50,0},
	{1,51,0},
	{1,52,0},
	{1,53,0},
	{1,54,0},
	{1,55,0},
	{1,56,0},
	{1,57,0},
	{1,58,0},
	{1,59,0},
	{1,60,0},
	{1,61,0},
	{1,62,0},
	{1,63,0},
	{1,64,0},
	{1,65,0},
	{1,66,0},
	{1,67,0},
	{1,68,0},
	{1,69,0},
	{1,70,0},
	{1,71,0},
	{1,72,0},
	{1,73,0},
	{1,74,0},
	{1,75,0},
	{1,76,0},
	{1,77,0},
	{1,78,0},
	{1,79,0},
	{1,80,0},
	{1,81,0},
	{1,82,0},
	{1,83,0},
	{1,84,0},
	{1,85,0},
	{1,86,0},
	{1,87,0},
	{1,88,0},
	{1,89,0},
	{1,90,0},
	{1,91,0},
	{1,92,0},
	{1,93,0},
	{1,94,0},
	{1,95,0},
	{1,96,0},
	{0,-1,3},
	{1,-1,3},
	{-1,0,0}
};
Gecode::DFA::Transition big[] = {
	{0,1,0},
	{0,2,0},
	{0,3,1},
	{0,4,0},
	{0,5,0},
	{0,6,0},
	{0,7,0},
	{0,8,0},
	{0,9,0},
	{0,10,0},
	{0,11,0},
	{0,12,0},
	{0,13,0},
	{0,14,0},
	{0,15,0},
	{0,16,0},
	{0,17,0},
	{0,18,0},
	{0,19,0},
	{0,20,0},
	{0,21,0},
	{0,22,0},
	{0,23,0},
	{0,24,0},
	{0,25,0},
	{0,26,0},
	{0,27,0},
	{0,28,0},
	{0,29,0},
	{0,30,0},
	{0,31,0},
	{0,32,0},
	{0,33,0},
	{0,34,0},
	{0,35,0},
	{0,36,0},
	{0,37,0},
	{0,38,0},
	{0,39,0},
	{0,40,0},
	{0,41,0},
	{0,42,0},
	{0,43,0},
	{0,44,0},
	{0,45,0},
	{0,46,0},
	{0,47,0},
	{0,48,0},
	{0,49,0},
	{0,50,0},
	{0,51,0},
	{0,52,0},
	{0,53,0},
	{0,54,0},
	{0,55,0},
	{0,56,0},
	{0,57,0},
	{0,58,0},
	{0,59,0},
	{0,60,0},
	{0,61,0},
	{0,62,0},
	{0,63,0},
	{0,64,0},
	{0,65,0},
	{0,66,0},
	{0,67,0},
	{0,68,0},
	{0,69,0},
	{0,70,0},
	{0,71,0},
	{0,72,0},
	{0,73,0},
	{0,74,0},
	{0,75,0},
	{0,76,0},
	{0,77,0},
	{0,78,0},
	{0,79,0},
	{0,80,0},
	{0,81,0},
	{0,82,0},
	{0,83,0},
	{0,84,0},
	{0,85,0},
	{0,86,0},
	{0,87,0},
	{0,88,0},
	{0,89,0},
	{0,90,0},
	{0,91,0},
	{0,92,0},
	{0,93,0},
	{0,94,0},
	{0,95,0},
	{0,96,0},
	{1,1,0},
	{1,2,0},
	{1,3,0},
	{1,4,0},
	{1,5,0},
	{1,6,0},
	{1,7,0},
	{1,8,2},
	{1,9,0},
	{1,10,0},
	{1,11,0},
	{1,12,0},
	{1,13,0},
	{1,14,0},
	{1,15,0},
	{1,16,0},
	{1,17,0},
	{1,18,0},
	{1,19,0},
	{1,20,0},
	{1,21,0},
	{1,22,0},
	{1,23,0},
	{1,24,0},
	{1,25,0},
	{1,26,0},
	{1,27,0},
	{1,28,0},
	{1,29,0},
	{1,30,0},
	{1,31,0},
	{1,32,0},
	{1,33,0},
	{1,34,0},
	{1,35,0},
	{1,36,0},
	{1,37,0},
	{1,38,0},
	{1,39,0},
	{1,40,0},
	{1,41,0},
	{1,42,0},
	{1,43,0},
	{1,44,0},
	{1,45,0},
	{1,46,0},
	{1,47,0},
	{1,48,0},
	{1,49,0},
	{1,50,0},
	{1,51,0},
	{1,52,0},
	{1,53,0},
	{1,54,0},
	{1,55,0},
	{1,56,0},
	{1,57,0},
	{1,58,0},
	{1,59,0},
	{1,60,0},
	{1,61,0},
	{1,62,0},
	{1,63,0},
	{1,64,0},
	{1,65,0},
	{1,66,0},
	{1,67,0},
	{1,68,0},
	{1,69,0},
	{1,70,0},
	{1,71,0},
	{1,72,0},
	{1,73,0},
	{1,74,0},
	{1,75,0},
	{1,76,0},
	{1,77,0},
	{1,78,0},
	{1,79,0},
	{1,80,0},
	{1,81,0},
	{1,82,0},
	{1,83,0},
	{1,84,0},
	{1,85,0},
	{1,86,0},
	{1,87,0},
	{1,88,0},
	{1,89,0},
	{1,90,0},
	{1,91,0},
	{1,92,0},
	{1,93,0},
	{1,94,0},
	{1,95,0},
	{1,96,0},
	{-1,0,0}
};

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
	IntVar var_0xINPUT_2_n, T0_2_n, T1_2_n, T2_2_n, T3_2_n, PCTEMP_LHS_1;
	IntVarArray var_0xINPUT_2, T0_2, T1_2, T2_2, T3_2;
	DFA notjohn;
public:
	enum {BRANCH_A_N, BRANCH_N_A, SEARCH_DFS, PROP_OPEN, PROP_PAD};
	KaluzaExample(const SizeOptions& opt)
	: var_0xINPUT_2_n(*this,0,wordlength),
T0_2_n(*this,0,wordlength),
T1_2_n(*this,0,wordlength),
T3_2_n(*this,0,wordlength),
PCTEMP_LHS_1(*this,0,wordlength),
var_0xINPUT_2(*this,wordlength,val_dom_min,val_dom_max),
T0_2(*this,wordlength,val_dom_min,val_dom_max),
T1_2(*this,wordlength,val_dom_min,val_dom_max),
T3_2(*this,wordlength,val_dom_min,val_dom_max){
	// constant strings
	string str;
	//T2_2 \in CapturedBrack(/["']/, 0);
	CSTRING(T2_2, T2_2_n, "\"'");
	//PCTEMP_LHS_1 == Len(T0_2);
	rel(*this, PCTEMP_LHS_1==T0_2_n);
	//PCTEMP_LHS_1 == 0x6;
	rel(*this, PCTEMP_LHS_1==6);
		
	switch(opt.propagation()){
		case PROP_PAD:
		{
			PAD_INVARIANT(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			PAD_INVARIANT(*this,T0_2, T0_2_n);
			PAD_INVARIANT(*this,T1_2, T1_2_n);
			PAD_INVARIANT(*this,T2_2, T2_2_n);
			PAD_INVARIANT(*this,T3_2, T3_2_n);
			
			//var_0xINPUT_2 := T0_2 . T1_2;
			rel(*this, var_0xINPUT_2_n == (T0_2_n+T1_2_n)); 
			for(int i=0; i<wordlength; i++) {
				rel(*this, (i<T0_2_n) >> (T0_2[i]==var_0xINPUT_2[i]));	
				BoolVar b = expr(*this, i==T0_2_n);						
				for(int j=0; (i+j)<wordlength; j++)
					rel(*this, (b&&(j<T1_2_n)) >> (T1_2[j]==var_0xINPUT_2[i+j]));	
			}
			//T1_2 := T2_2 . T3_2;
			rel(*this, T1_2_n == (T2_2_n+T3_2_n)); 
			for(int i=0; i<wordlength; i++) {
				rel(*this, (i<T2_2_n) >> (T2_2[i]==T1_2[i]));	
				BoolVar b = expr(*this, i==T2_2_n);						
				for(int j=0; (i+j)<wordlength; j++)
					rel(*this, (b&&(j<T3_2_n)) >> (T3_2[j]==T1_2[i+j]));	
			}
			//T0_2 \notin CapturedBrack(/["']/, 0);
			int fp[] = {0,1,-1};
			notjohn = DFA(0,bigp,fp);
			extensional(*this, T0_2, notjohn);
			break;
		}
		case PROP_OPEN:
		{
			open_invariant(*this,var_0xINPUT_2, var_0xINPUT_2_n);
			open_invariant(*this,T0_2, T0_2_n);
			open_invariant(*this,T1_2, T1_2_n);
			open_invariant(*this,T2_2, T2_2_n);
			open_invariant(*this,T3_2, T3_2_n);


			//var_0xINPUT_2 := T0_2 . T1_2;
			//T1_2 := T2_2 . T3_2;
			open_concat(*this, T0_2, T0_2_n, T1_2, T1_2_n, var_0xINPUT_2, var_0xINPUT_2_n);
			open_concat(*this, T2_2, T2_2_n, T3_2, T3_2_n, T1_2, T1_2_n);
			//T0_2 \notin CapturedBrack(/["']/, 0);
			int fp[] = {0,1,-1};
			notjohn = DFA(0,bigp,fp);
			extensional(*this, T0_2, notjohn);
			// 				int f[] = {0,1,-1};
			// 				notjohn = DFA(0,big,f2);
			// 				extensional(*this, T0_2, notjohn, T0_2_n);
			break;
		}
	}
		
				branch(*this, var_0xINPUT_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T0_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T1_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T2_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, T3_2, INT_VAR_SIZE_MIN(), INT_VAL_MIN());


	}
	
	KaluzaExample(bool share, KaluzaExample& s)
	: Script(share,s) {
								var_0xINPUT_2.update(*this, share, s.var_0xINPUT_2);
			T0_2.update(*this, share, s.T0_2);
			T1_2.update(*this, share, s.T1_2);
			T2_2.update(*this, share, s.T2_2);
			T3_2.update(*this, share, s.T3_2);
						var_0xINPUT_2_n.update(*this, share, s.var_0xINPUT_2_n);
			T0_2_n.update(*this, share, s.T0_2_n);
			T1_2_n.update(*this, share, s.T1_2_n);
			T2_2_n.update(*this, share, s.T2_2_n);
			T3_2_n.update(*this, share, s.T3_2_n);
			PCTEMP_LHS_1.update(*this, share, s.PCTEMP_LHS_1);
			notjohn.update(*this, share, s.notjohn);

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




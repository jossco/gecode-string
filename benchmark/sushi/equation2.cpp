/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./benchmark/sushi/equation2.cpp
version: 	0.2.1
date: 		Wed Dec  3 17:58:46 CET 2014
========
Constraint model by Jun He
*/

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

#include <gecode/driver.hh>
#include <gecode/minimodel.hh>

#include "bounded-none.hh"
#include "open.hh"
#include "open-layered-graph.hh"

using namespace Gecode;

using namespace std;

int wordlength = 0;
int dummy_sym = 0;
int val_dom_min = 0;
int val_dom_max = 2;
bool gecode_find_solution;

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

/*  The string equation is x[n,2n] = a{n,n}
 *  The CP modelling is as following:
 *  x in [ab]*
 *  x[n,2n] = a{n,n}
 */

class SUSHI_EQUATION : public Script {

	IntVar n_x;
	IntVarArray X;

public:
  enum {MODEL_SIMPLE,MODEL_SUBSTRING,BRANCH_A_N, BRANCH_N_A, BRANCH_FILTER, BRANCH_BOUND, SEARCH_ITERATE, SEARCH_DFS, PROP_OPEN, PROP_CLOSED, PROP_PAD};
	SUSHI_EQUATION(const SizeOptions& opt)
	: n_x(*this, 1, wordlength),
	  X(*this, wordlength , val_dom_min, val_dom_max){

      int n = opt.size();
		gecode_find_solution = false;

    REG r_ab(IntArgs(2, 1,2));
		REG r_a(1);
		REG dum(dummy_sym);
		REG reg_x = *r_ab;
    REG reg_a = r_a(n,n);
    if (opt.propagation() == PROP_PAD) {
      reg_x += (*dum);
    }
    
	  DFA myDFA_x(reg_x);
    DFA myDFA_a(reg_a);
    IntVarArgs tempVar;
    switch(opt.propagation()){
      case PROP_OPEN:
        extensional(*this, X, myDFA_x, n_x);
    		for(int i=n; i<2*n && i < X.size(); i++)
    		{
    			tempVar << X[i];
    		}
  	    extensional(*this, tempVar, myDFA_a);
        break;
      case PROP_CLOSED:
        rel(*this, n_x>=2*n);
        extensional(*this, X, myDFA_x);
    		for(int i=n; i<2*n && i<X.size(); i++)
    		{
    			tempVar << X[i];
    		}
  	    extensional(*this, tempVar, myDFA_a);
        break;
      case PROP_PAD:
        extensional(*this, X, myDFA_x);
    		for(int i=n; i<2*n && i < X.size(); i++)
    		{
    			tempVar << X[i];
    		}
  	    extensional(*this, tempVar, myDFA_a);
  	    for(int i=0; i<wordlength; i++)
  	    {
  	    	rel(*this, (X[i]==dummy_sym) == (n_x<=i));
  	    }
        break;
    }
	  
    IntVarArgs lengths;
		lengths << n_x;
		
    switch(opt.branching()){
    case BRANCH_N_A:
      branch(*this, lengths, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCH_A_N:
		  branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      branch(*this, lengths, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCH_BOUND:
      boundednone(*this, X, n_x);
      break;
    }
	}

	~SUSHI_EQUATION()
	{

	}


	SUSHI_EQUATION(bool share, SUSHI_EQUATION& s)
    : Script(share,s)
	{
		X.update(*this, share, s.X);
		n_x.update(*this, share, s.n_x);
	}

	virtual Space* copy(bool share)
	{
		return new SUSHI_EQUATION(share,*this);
	}

	virtual void print(std::ostream& os) const
	{
		os << std::endl << "Find the following solution X with string length "
				<< n_x.val() <<  ":" << std::endl;
		os << "\"";

		gecode_find_solution = true;

		for(int i=0; i<n_x.val(); i++)
		{
			os << X[i];
		}

		os << "\"" << std::endl;
	}
};

int main(int argc, char* argv[]) {

/*	if(argc!=2)
	{
		cout << "Wrong parameters for running this program!!!" << endl;
		return 0;
	}
*/

	ofstream resFile;
	resFile.open("experiments/SUSHI/eq2.res");

	gecode_find_solution = true;

	int end_time1 = clock();

	SizeOptions opt("SUSHI-EQUATION");
	opt.solutions(1); // only need one solution
  
  opt.size(37);
  
  opt.propagation(SUSHI_EQUATION::PROP_OPEN,   "open",    "bounded-length extensional propagation");
  opt.propagation(SUSHI_EQUATION::PROP_PAD,    "pad",     "fixed, maximal length string with padding characters");
  opt.propagation(SUSHI_EQUATION::PROP_CLOSED, "closed",  "fixed-length extensional propagation");
  opt.propagation(SUSHI_EQUATION::PROP_OPEN);
  
  opt.search(SUSHI_EQUATION::SEARCH_DFS,       "dfs",     "dfs search");
  opt.search(SUSHI_EQUATION::SEARCH_ITERATE,   "iterate", "search each possible length in turn");
  opt.search(SUSHI_EQUATION::SEARCH_DFS);
  
  opt.branching(SUSHI_EQUATION::BRANCH_A_N,    "an",      "branch array, then length");
  opt.branching(SUSHI_EQUATION::BRANCH_N_A,    "na",      "branch length, then array");
  opt.branching(SUSHI_EQUATION::BRANCH_FILTER, "filter",  "filter to branch on characters under min length");
  opt.branching(SUSHI_EQUATION::BRANCH_BOUND,  "bound",   "custom brancher to branch on characters under min length");
  opt.branching(SUSHI_EQUATION::BRANCH_BOUND);
  
	opt.parse(argc,argv);
  wordlength = 4*opt.size();
	cout << endl
		 << "We are now solving the equation 1 with n = " << opt.size() << endl;
  switch(opt.search()){
    case SUSHI_EQUATION::SEARCH_DFS:
	    Script::run<SUSHI_EQUATION,DFS,SizeOptions>(opt);
      break;
    case SUSHI_EQUATION::SEARCH_ITERATE:
      std::ofstream sol_file;
      std::ostream& s_out = select_ostream(opt.out_file(), sol_file);
      Support::Timer t;
      t.start();
      Search::Statistics stat;
      Search::Options sopt;
      sopt.threads = opt.threads();
      sopt.c_d = opt.c_d();
      int solutions = 0;
      int maxwordlength = wordlength;
      for (wordlength= 1; wordlength <= maxwordlength; wordlength++ ) {
        SUSHI_EQUATION* m = new SUSHI_EQUATION(opt);
        DFS<SUSHI_EQUATION> e(m,sopt);
        delete m;
        while (SUSHI_EQUATION* s = e.next()) {
          s->print(s_out); delete s;
          if (++solutions == opt.solutions())
            break;
        }
        stat += e.statistics();
        if (opt.solutions() != 0 && solutions >= opt.solutions())
          break;
      }
      std::cout << "Summary: " << std::endl
                << "\truntime:\t";
      Driver::stop(t, std::cout);
      std::cout << "\n\tsolutions:\t"    << solutions << std::endl
                << "\tpropagations:\t" << stat.propagate << std::endl
                << "\tnodes:\t\t"        << stat.node << std::endl
                << "\tfailures:\t"     << stat.fail << std::endl
                << "\trestarts:\t"     << stat.restart << std::endl
                << "\tno-goods:\t"     << stat.nogood << std::endl
                << "\tpeak depth:\t"   << stat.depth << std::endl;
      break;
  }

	int end_time2 = clock();

	if(gecode_find_solution)
	{
		resFile << 1 << endl;
	}
	else resFile << 0 << endl;

	resFile << end_time2*1000.0f/CLOCKS_PER_SEC << endl;

	cout << endl
	     << "The instance is now solved in " << end_time2*1000.0f/CLOCKS_PER_SEC
	     << ", with " << (end_time2-end_time1)*1000.0f/CLOCKS_PER_SEC
	     << " for constraint solving" << endl;

	return 0;
}




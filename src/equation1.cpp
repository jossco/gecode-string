/*
Author:   Joseph D. Scott
          Uppsala University

Used in:
          "On Constraint Solving with String Variables"
          Scott, J.D., Flener, P. and Pearson, J.
          submitted to CP 2014

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

/*  The string equation is x + a{n,n} = (a|b){2n,2n}
 *  The CP modelling is as following:
 *  Z = X + Y
 *  Z \in (a|b){2n,2n}
 *  Y \in a{n,n}
 */

class SUSHI_EQUATION : public Script {

	IntVar n_x, n_y, n_z;
	IntVarArray X, Y, Z;

public:
  enum {BRANCH_A_N, BRANCH_N_A, BRANCH_FILTER, BRANCH_BOUND, SEARCH_ITERATE, SEARCH_DFS, PROP_OPEN, PROP_CLOSED, PROP_PAD};
	SUSHI_EQUATION(const SizeOptions& opt)
	: n_x(*this, 1, wordlength), n_y(*this, 1, wordlength), n_z(*this, 1, wordlength),
	  X(*this, wordlength , val_dom_min, val_dom_max),
	  Y(*this, wordlength , val_dom_min, val_dom_max),
	  Z(*this, wordlength , val_dom_min, val_dom_max){

      int n = opt.size();
		gecode_find_solution = false;

		REG r_ab(IntArgs(2, 1,2));
    REG r_a(1);
		REG dum(dummy_sym);
    REG reg_y = r_a(n,n);
    REG reg_z = r_ab(2*n,2*n);
    if (opt.propagation() == PROP_PAD) {
      reg_y += (*dum);
      reg_z += (*dum);
    }
    DFA myDFA_y(reg_y);
	  DFA myDFA_z(reg_z);
    switch(opt.propagation()){
      case PROP_CLOSED:
        extensional(*this, Z, myDFA_z);
        extensional(*this, Y, myDFA_y);
        rel(*this, n_z == (n_x+n_y));
  	    for(int i=0; i<wordlength; i++)
  	    {
  	    	rel(*this, (i<n_x) >> (X[i]==Z[i]));

  	    	BoolVar b = expr(*this, i==n_x);

  	    	for(int j=0; (i+j)<wordlength; j++)
  	    	{
  	    		rel(*this, (b&&(j<n_y)) >> (Y[j]==Z[i+j]));
  	    	}
  	    }
        break;
      case PROP_PAD:
        extensional(*this, Z, myDFA_z);
        extensional(*this, Y, myDFA_y);
  	    for(int i=0; i<wordlength; i++)
  	    {
  	    	rel(*this, (Z[i]==dummy_sym) == (n_z<=i));
  	    	rel(*this, (Y[i]==dummy_sym) == (n_y<=i));
  	    	rel(*this, (X[i]==dummy_sym) == (n_x<=i));
  	    }
        rel(*this, n_z == (n_x+n_y));
  	    for(int i=0; i<wordlength; i++)
  	    {
  	    	rel(*this, (i<n_x) >> (X[i]==Z[i]));

  	    	BoolVar b = expr(*this, i==n_x);

  	    	for(int j=0; (i+j)<wordlength; j++)
  	    	{
  	    		rel(*this, (b&&(j<n_y)) >> (Y[j]==Z[i+j]));
  	    	}
  	    }
        break;
      case PROP_OPEN:
        extensional(*this, Z, myDFA_z, n_z);
        extensional(*this, Y, myDFA_y, n_y);
        concat(*this,X, n_x, Y, n_y, Z, n_z,ICL_BND);
        rel(*this, n_z == 2 * n_y);
        break;
    }
	  
    IntVarArgs lengths;
		lengths << n_z << n_y << n_x;
		
    switch(opt.branching()){
    case BRANCH_N_A:
      branch(*this, lengths, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, Z, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, Y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCH_A_N:
		  branch(*this, Z, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, Y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		  branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      branch(*this, lengths, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCH_BOUND:
      boundednone(*this, Y, n_y);
      boundednone(*this, Z, n_z);
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
		Z.update(*this, share, s.Z);
		Y.update(*this, share, s.Y);
		X.update(*this, share, s.X);

		n_z.update(*this, share, s.n_z);
		n_y.update(*this, share, s.n_y);
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

		os << std::endl << "Find the following solution Y with string length "
				<< n_y.val() <<  ":" << std::endl;
		os << "\"";

		for(int i=0; i<n_y.val(); i++)
		{
			os << Y[i];
		}

		os << "\"" << std::endl;

		os << std::endl << "Find the following solution Z with string length "
				<< n_z.val() <<  ":" << std::endl;
		os << "\"";

		for(int i=0; i<n_z.val(); i++)
		{
			os << Z[i];
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
	resFile.open("experiments/SUSHI/eq1.res");

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




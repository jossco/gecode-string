/*
 * reg2gecode.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: jun
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

using namespace Gecode;

using namespace std;

int n;
int val_dom_min = 0;
int val_dom_max = 2;
bool gecode_find_solution;

class SUSHI_EQUATION : public Script {

	IntVarArray dec_variables;

public:

	SUSHI_EQUATION(const Options& opt)
	: dec_variables(*this, 2*n , val_dom_min, val_dom_max) {

		gecode_find_solution = false;

		REG r_a(1);
		REG r_a_plus = +r_a;

		REG r_eps(0);
		REG r_eps_star = *r_eps;

		REG r_b_plus[n];
		REG r_b(2);

		for(int i=0; i<n; i++)
		{
			r_b_plus[i] = r_b(i+1, i+1);
		}

		REG r = r_a_plus + r_b_plus[n-1];
		for(int i=0; i<n; i++)
		{
			if(i<(n-1)) r = r | (r_b_plus[i] + r_a_plus + r_b_plus[n-i-2]);
			else  r = r | (r_b_plus[i] + r_a_plus);
		}

	    DFA myDFA(r+r_eps_star);
	    extensional(*this, dec_variables, myDFA);

		branch(*this, dec_variables, INT_VAR_SIZE_MIN, INT_VAL_MIN);
	}

	~SUSHI_EQUATION()
	{

	}


	SUSHI_EQUATION(bool share, SUSHI_EQUATION& s)
    : Script(share,s)
	{
		dec_variables.update(*this, share, s.dec_variables);
	}

	virtual Space* copy(bool share)
	{
		return new SUSHI_EQUATION(share,*this);
	}

	virtual void print(std::ostream& os) const
	{
		os << std::endl << "Find the following solution:" << std::endl;
		os << "\"";

		gecode_find_solution = true;

		for(int i=0; i<2*n; i++)
		{
			os << dec_variables[i];
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

	n = atoi(argv[1]);*/

	n = 37;

	cout << endl
		 << "We are now solving the equation 4 with n = " << n << endl;

	ofstream resFile;
	resFile.open("/home/jun/workspace/SUSHI_EQ/experiments/eq4.res");

	gecode_find_solution = true;

	int end_time1 = clock();

	Options opt("SUSHI-EQUATION");
	opt.solutions(1); // only need one solution
	opt.parse(argc,argv);

	Script::run<SUSHI_EQUATION,DFS,Options>(opt);

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




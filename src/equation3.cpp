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
int dummy_sym = 0;
int val_dom_min = 0;
int val_dom_max = 2;
bool gecode_find_solution;

/*  The string equation is x + a + y[0,n] = b{n,n}ab{n,n}
 *  The CP modelling is as following:
 *  Z = X + T
 *  T = a + Y[0,n]
 *  Z \in b{n,n}ab{n,n}
 */

class SUSHI_EQUATION : public Script {

	int n_up;
	IntVar n_x, n_y, n_z, n_t;
	IntVarArray X, Y, Z, T;

public:

	SUSHI_EQUATION(const Options& opt)
	: n_up(4*n), n_x(*this, 0, n_up), n_y(*this, 0, n_up), n_z(*this, 0, n_up),
	  n_t(*this, n+1, n+1),
	  X(*this, n_up , val_dom_min, val_dom_max),
	  Y(*this, n_up , val_dom_min, val_dom_max),
	  Z(*this, n_up , val_dom_min, val_dom_max),
	  T(*this, n_up , val_dom_min, val_dom_max){

		gecode_find_solution = false;

		REG r_b(2);
		REG r_a(1);
		REG dum(dummy_sym);
		REG reg_z = r_b(n,n) + r_a + r_b(n,n) + (*dum);

	    DFA myDFA(reg_z);
	    extensional(*this, Z, myDFA);

	    rel(*this, T[0] == 1);

	    for(int i=0; i<n; i++)
	    {
	    	T[i+1] = Y[i];
	    }

	    for(int i=0; i<n_up; i++)
	    {
	    	rel(*this, (Z[i]==dummy_sym) == (n_z<=i));
	    	rel(*this, (Y[i]==dummy_sym) == (n_y<=i));
	    	rel(*this, (X[i]==dummy_sym) == (n_x<=i));
	    	rel(*this, (T[i]==dummy_sym) == (n_t<=i));
	    }

	    rel(*this, n_z == (n_x+n_t));

	    for(int i=0; i<n_up; i++)
	    {
	    	rel(*this, (i<n_x) >> (X[i]==Z[i]));

	    	BoolVar b = expr(*this, i==n_x);

	    	for(int j=0; (i+j)<n_up; j++)
	    	{
	    		rel(*this, (b&&(j<n_t)) >> (T[j]==Z[i+j]));
	    	}
	    }

		IntVarArgs tempVar;
		tempVar << n_z << n_y << n_x << n_t;
		branch(*this, tempVar, INT_VAR_SIZE_MIN, INT_VAL_MIN);

		branch(*this, Z, INT_VAR_SIZE_MIN, INT_VAL_MIN);
		branch(*this, Y, INT_VAR_SIZE_MIN, INT_VAL_MIN);
		branch(*this, X, INT_VAR_SIZE_MIN, INT_VAL_MIN);
		branch(*this, T, INT_VAR_SIZE_MIN, INT_VAL_MIN);
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
		T.update(*this, share, s.T);

		n_z.update(*this, share, s.n_z);
		n_y.update(*this, share, s.n_y);
		n_x.update(*this, share, s.n_x);
		n_t.update(*this, share, s.n_t);
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

		os << std::endl << "Find the following solution T with string length "
				<< n_t.val() <<  ":" << std::endl;
		os << "\"";

		for(int i=0; i<n_t.val(); i++)
		{
			os << T[i];
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

	n = atoi(argv[1]);*/

	n = 37;

	cout << endl
		 << "We are now solving the equation 3 with n = " << n << endl;

	ofstream resFile;
	resFile.open("/home/jun/workspace/SUSHI_EQ/experiments/eq3.res");

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




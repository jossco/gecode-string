#include <gecode/int.hh>
#include <gecode/search.hh>
#include "open.hh"
#include <iostream>

#define MAXSIZE 10

using namespace Gecode;

// base test class
class TestInvariant : public Space {
protected:
	IntVar Xn;
  IntVarArray X;
public:
	std::string description;
public:
	virtual bool passed(void) const {return true;}
	virtual bool shouldPass(void) const {return true;}
	TestInvariant(const char* desc, int xmin, int xmax, int size)
	: Xn(*this,xmin, xmax),
	X(*this, size),
	description(desc) {
		IntSet charSet = Open::OpenString::paddedIntSet(0,95);
		X = IntVarArray(*this, size, charSet);

		// invariant constraints
		open_invariant(*this, X, Xn);
	}
	void boilerplate(void) {
		branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
  // search support
  TestInvariant(bool share, TestInvariant& s) : Space(share, s) {
    X.update(*this, share, s.X);
		Xn.update(*this, share, s.Xn);
	}
  virtual Space* copy(bool share) {
    return new TestInvariant(share,*this);
  }
  // print solution
  void print(void) const {
    std::cout << "X = ";
		Open::OpenString::print(std::cout,X,Xn);
    std::cout << std::endl;
  }
};


// length should affect contents, and vice versa
class TestMaxToExc : public TestInvariant {
public:
  TestMaxToExc(void) 
	: TestInvariant("max len prunes the excluded region",0, 3, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		bool passed = true;
		for (int i = 3; i < MAXSIZE; i++)
			if (!X[i].assigned() || X[i].val() != Open::OpenString::padchar)
				passed = false;
		return passed;
	}
};

class TestMinToMan : public TestInvariant {
public:
  TestMinToMan(void) 
	: TestInvariant("min len prunes the mandatory region",3, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		bool passed = true;
		for (int i = 0; i < 3; i++)
			if (X[i].in(Open::OpenString::padchar))
				passed = false;
		return passed;
	}
};

class TestTooLong : public TestInvariant {
public:
  TestTooLong(void) 
	: TestInvariant("max length longer than X.size()", 0, MAXSIZE*2, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		return Xn.max()==MAXSIZE;
	}
};

class TestNegLen : public TestInvariant {
public:
  TestNegLen(void) 
	: TestInvariant("min length negative", -5, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		return Xn.min()==0;
	}
};

class TestManToMin : public TestInvariant {
public:
  TestManToMin(void) 
	: TestInvariant("removing padchar affects min length", 0, MAXSIZE, MAXSIZE) {
		IntSet dom_x(1,95);
		dom(*this, X[3], dom_x);
		boilerplate();
  }
	bool passed(void) const {
		return Xn.min()==4;
	}
};

class TestExcToMax : public TestInvariant {
public:
  TestExcToMax(void) 
	: TestInvariant("assigning padchar affects max length", 0, MAXSIZE, MAXSIZE) {
		IntSet dom_x(Open::OpenString::padchar,Open::OpenString::padchar);
		dom(*this, X[5], dom_x);
		boilerplate();
  }
	bool passed(void) const {
		return Xn.max()==5;
	}
};

class TestMandatory : public TestInvariant {
public:
  TestMandatory(void) 
	: TestInvariant("mandatory position is assigned to padchar", 3, MAXSIZE, MAXSIZE) {
		IntSet dom_x(96,96);
		dom(*this, X[2], dom_x);
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};

// Should fail if an excluded position does not include padchar
class TestExcluded : public TestInvariant {
public:
  TestExcluded(void) 
	: TestInvariant("", 0, MAXSIZE-3, MAXSIZE) {
		IntSet domx(1,95);
		dom(*this, X[MAXSIZE-2],domx);
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};

int passCount = 0;
int runCount = 0;
void runTest(TestInvariant* test) {
	bool pass=true;
	SpaceStatus stat = test->status();
	std::cout << "Test - " << test->description << std::endl;
	if (test->passed()) {
		if (test->shouldPass()){
			if(stat == SS_FAILED) {
				std::cout << "FAIL (space failed)";
				pass = false;
			} else {
				std::cout << "PASS (space not failed)";
			}
		} else {
			if(stat != SS_FAILED) {
				std::cout << "FAIL (space should have failed)";
				pass = false;
			} else {
				std::cout << "PASS (space failed)";
			}
		}
	} else {
		pass = false;
		std::cout << "FAIL (bad pruning?)";
		if(stat == SS_FAILED) {
			std::cout << " (space failed)";
		} else {
			std::cout << " (space not failed)";
		}
	}
	std::cout << std::endl;
	test->print();
	std::cout << "\n=======\n";
	runCount++;
	if (pass) passCount++;
}

// main function
int main(int argc, char* argv[]) {
  TestMaxToExc* m1 = new TestMaxToExc;
	runTest(m1);
  TestExcluded* m2 = new TestExcluded;
	runTest(m2);
  TestMandatory* m3 = new TestMandatory;
	runTest(m3);
	TestMinToMan* m4 = new TestMinToMan;
	runTest(m4);
	TestExcToMax* m5 = new TestExcToMax;
	runTest(m5);
	TestManToMin* m6 = new TestManToMin;
	runTest(m6);
	TestTooLong* m7 = new TestTooLong;
	runTest(m7);
	TestNegLen* m8 = new TestNegLen;
	runTest(m8);
  
	std::cout << passCount << " out of " << runCount << " tests passed." << std::endl;
  return 0;
}

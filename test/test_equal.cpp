#include <gecode/int.hh>
#include <gecode/search.hh>
#include "open.hh"
#include <iostream>

#define MAXSIZE 10

using namespace Gecode;

// base test class
class TestEqual : public Space {
protected:
	IntVar Xn, Yn;
  IntVarArray X, Y;
public:
	std::string description;
public:
	virtual bool passed(void) const {return true;}
	virtual bool shouldPass(void) const {return true;}
	TestEqual(const char* desc, int xmin, int xmax, int ymin, int ymax, int size)
	: Xn(*this,xmin, xmax), Yn(*this, ymin, ymax),
	X(*this, size), Y(*this, size),
	description(desc) {
		IntSet charSet = Open::OpenString::paddedIntSet(1,95);
		X = IntVarArray(*this, size, charSet);
		Y = IntVarArray(*this, size, charSet);

		// invariant constraints
		open_invariant(*this, X, Xn);
		open_invariant(*this, Y, Yn);
	}

	void boilerplate(void) {
		open_equal(*this, X, Xn, Y, Yn);
    // post branching
    branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, Y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
  // search support
  TestEqual(bool share, TestEqual& s) : Space(share, s) {
    X.update(*this, share, s.X);
		Xn.update(*this, share, s.Xn);
		Y.update(*this, share, s.Y);
		Yn.update(*this, share, s.Yn);
	}
  virtual Space* copy(bool share) {
    return new TestEqual(share,*this);
  }
  // print solution
  void print(void) const {
    std::cout << "X = ";
		Open::OpenString::print(std::cout,X,Xn);
    std::cout << std::endl<< "Y = ";
		Open::OpenString::print(std::cout,Y,Yn);
		std::cout << std::endl;
  }
};

// non-overlapping lengths
class TestLengthMismatch : public TestEqual {
public:
  TestLengthMismatch(void) 
	: TestEqual("non-overlapping length domains" ,0, 3, MAXSIZE-3, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};
class TestSingleLength : public TestEqual {
public:
  TestSingleLength(void) 
	: TestEqual("one possible length" ,0, 3, 3, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return true;}
};
class TestTooLong : public TestEqual {
public:
  TestTooLong(void) 
	: TestEqual("length doms bigger than actual arrays" ,0, MAXSIZE*2, 0, MAXSIZE*2, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return true;}
};

class TestDisjunctCharacter : public TestEqual {
public:
	bool passed(void) const {
		bool pass = true;
		for (int i = 5; i < MAXSIZE; i++) {
			if (!X[i].assigned() || X[i].val() != Open::OpenString::padchar ||
				  !Y[i].assigned() || Y[i].val() != Open::OpenString::padchar )
						pass = false;
		}
		return (pass && Xn.max() ==5 && Yn.max() == 5);
	}
  TestDisjunctCharacter(void) 
	: TestEqual("", 3, MAXSIZE-3, 3, MAXSIZE-3, MAXSIZE) {
		IntArgs x(6,Open::OpenString::padchar, 1, 3, 5, 7, 9);
		IntSet xSet(x);
		dom(*this, X[5], xSet);
		IntArgs y(6,Open::OpenString::padchar, 0, 2, 4, 6, 8);
		IntSet ySet(y);
		dom(*this, Y[5], ySet);
		boilerplate();
	}
};

class TestDualChannel : public TestEqual {
public:
	bool passed(void) const {
		bool pass = true;
		for (int i = 7; i < MAXSIZE; i++) {
			if (!X[i].assigned() || X[i].val() != Open::OpenString::padchar ||
				  !Y[i].assigned() || Y[i].val() != Open::OpenString::padchar )
						pass = false;
		}
		return (pass && Xn.max() ==7 && Yn.max() == 7 &&
						Yn.min() == 4 && Xn.min() == 4);
	}
  TestDualChannel(void) 
	: TestEqual("length-content channeling over multiple strings",0, MAXSIZE-3, 3, MAXSIZE, MAXSIZE) {
		IntSet nopad( 1, 95);
		dom(*this, X[3], nopad);
		IntSet pad(Open::OpenString::padchar,Open::OpenString::padchar);
		dom(*this, Y[7],   pad);

		boilerplate();
  }
};

// length-content channeling over multiple strings
class TestSingleSolution : public TestEqual {
public:
	bool passed(void) const {
		bool pass = true;
		for (int i = 0; i < MAXSIZE; i++) {
			if (!X[i].assigned() || !Y[i].assigned() || X[i].val() != Y[i].val())
						pass = false;
		}
		return (pass && Xn.assigned() && Xn.val() == 6 &&
			              Yn.assigned() && Yn.val() == 6);
	}
  TestSingleSolution(void) 
	: TestEqual("only one satisfying string",0, 6, 6, MAXSIZE, MAXSIZE) {
		IntSet y0 = Open::OpenString::paddedIntSet(1,1);
		dom(*this,Y[0],y0);
		IntSet x1 = Open::OpenString::paddedIntSet(1,1);
		dom(*this,X[1],x1);
		IntSet xy2 = Open::OpenString::paddedIntSet(2,2);
		dom(*this,X[2],xy2);
		dom(*this,Y[2],xy2);
		IntArgs x(6, Open::OpenString::padchar, 1, 2, 3, 4, 5);
		IntSet xdom(x);
		IntArgs y(6,Open::OpenString::padchar, 0, 3, 6, 9, 12);
		IntSet ydom(y);
		dom(*this,X[3],xdom);
		dom(*this,Y[3],ydom);
		IntSet lowSet = Open::OpenString::paddedIntSet(0,4);
		IntSet highSet = Open::OpenString::paddedIntSet(4,12);
		dom(*this, X[4], lowSet);
		dom(*this, Y[4], highSet);
		dom(*this, X[5], highSet);
		dom(*this, Y[5], lowSet);
		
		boilerplate();
  }
};

int passCount = 0;
int runCount = 0;
void runTest(TestEqual* test) {
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
  TestLengthMismatch* m1 = new TestLengthMismatch;
	runTest(m1);
  TestDualChannel* m2 = new TestDualChannel;
	runTest(m2);
  TestSingleSolution* m3 = new TestSingleSolution;
	runTest(m3);
	TestDisjunctCharacter* m4 = new TestDisjunctCharacter;
	runTest(m4);
	TestSingleLength* m5 = new TestSingleLength;
	runTest(m5);
	TestTooLong* m6 = new TestTooLong;
	runTest(m6);
  
	std::cout << passCount << " out of " << runCount << " tests passed." << std::endl;
  return 0;
}

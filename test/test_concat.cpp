#include <gecode/int.hh>
#include <gecode/search.hh>
#include "open.hh"
#include <iostream>

#define MAXSIZE 10

using namespace Gecode;

// base class for tests:
class TestConcat : public Space {
protected:
	IntVar Xn, Yn, Zn;
  IntVarArray X, Y, Z;
public:
	std::string description;
public:
	virtual bool passed(void) const {return true;}
	virtual bool shouldPass(void) const {return true;}
	TestConcat(const char* desc, int xmin, int xmax, int ymin, int ymax, int zmin, int zmax, int size)
	: Xn(*this,xmin, xmax), Yn(*this, ymin, ymax), Zn(*this, zmin, zmax),
	X(*this, size), Y(*this, size), Z(*this, size),
	description(desc) {
		IntSet charSet = Open::OpenString::paddedIntSet(0,95);
		X = IntVarArray(*this, size, charSet);
		Y = IntVarArray(*this, size, charSet);
		Z = IntVarArray(*this, size, charSet);

		// invariant constraints
		open_invariant(*this, X, Xn);
		open_invariant(*this, Y, Yn);
		open_invariant(*this, Z, Zn);
  }
	void boilerplate(void) {
		open_concat(*this, X, Xn, Y, Yn, Z, Zn);
    // post branching
    branch(*this, X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, Y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		branch(*this, Z, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
  // search support
  TestConcat(bool share, TestConcat& s) : Space(share, s) {
    X.update(*this, share, s.X);
		Xn.update(*this, share, s.Xn);
		Y.update(*this, share, s.Y);
		Yn.update(*this, share, s.Yn);
		Z.update(*this, share, s.Z);
		Zn.update(*this, share, s.Zn);
	}
  virtual Space* copy(bool share) {
    return new TestConcat(share,*this);
  }
  // print solution
  void print(void) const {
    std::cout << "X = ";
		Open::OpenString::print(std::cout,X,Xn);
    std::cout << std::endl<< "Y = ";
		Open::OpenString::print(std::cout,Y,Yn);
    std::cout << std::endl<< "Z = ";
		Open::OpenString::print(std::cout,Z,Zn);
		std::cout << std::endl;
  }
};

class TestInvalidLengths : public TestConcat {
public:
  TestInvalidLengths(void) 
	: TestConcat("X has neg min, Ymax > Y.size", -5, MAXSIZE, 0, MAXSIZE*2, 0, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return true;}
};

class TestTooShort : public TestConcat {
public:
  TestTooShort(void) 
	: TestConcat("Xmin + Ymin > Zmax", MAXSIZE/2+1, MAXSIZE, MAXSIZE/2+1, MAXSIZE, 0, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};

class TestTooLong : public TestConcat {
public:
  TestTooLong(void) 
	: TestConcat("Xmax + Ymax < Zmin", 0, MAXSIZE/2-1, 0, MAXSIZE/2-1, MAXSIZE-1, MAXSIZE, MAXSIZE) {
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};

class TestLengthInv1 : public TestConcat {
public:
  TestLengthInv1(void) 
	: TestConcat("[2,4].[2,4]=[0,10]=>[2,4].[2,4]=[4,8]", 2, 4, 2, 4, 0, 10, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		return Xn.min()==2 && Xn.max()==4 && Yn.min()==2 && Yn.max()==4 && Zn.min()==4 && Zn.max()==8;
	}
};

class TestLengthInv2 : public TestConcat {
public:
  TestLengthInv2(void) 
	: TestConcat("[1,MAX].[4,MAX]=[4,8]=>[1,4].[4,7]=[5,8]", 1, MAXSIZE, 4, MAXSIZE, 4, 8, MAXSIZE) {
		boilerplate();
  }
	bool passed(void) const {
		return Xn.min()==1 && Xn.max()==4 && Yn.min()==4 && Yn.max()==7 && Zn.min()==5 && Zn.max()==8;
	}
};

class TestOptionalX : public TestConcat {
public:
  TestOptionalX(void) 
	: TestConcat("optional region of X: X[6] disjoint with Z[6]", 4, MAXSIZE, 4, MAXSIZE, 8, MAXSIZE, MAXSIZE) {
		IntArgs even(6,Open::OpenString::padchar,0,2,4,6,8);
		IntArgs  odd(6,Open::OpenString::padchar,1,3,5,7,9);
		IntSet evenSet(even);
		IntSet  oddSet(odd);
		dom(*this,X[6],evenSet);
		dom(*this,Z[6],oddSet);
		boilerplate();
  }
	bool passed(void) const {
		return Xn.max()==6 && Zn.max()==MAXSIZE && Yn.max()==MAXSIZE - 4
			  && Xn.min()==4 && Yn.min()==4       && Zn.min()==8;
	}
};

class TestMandatoryZ : public TestConcat {
public:
  TestMandatoryZ(void) 
	: TestConcat("values in mandatory range of Z", 3, 5, 5, 15, 8, 15, 15) {
		for (int i = 0; i < Xn.min(); i++) {
			IntSet dom_i(i+Yn.min(),i+Yn.min());
			dom(*this, X[i], dom_i);
		}
		for (int i = Xn.min(); i < Xn.max(); i++) {
			IntArgs d(2,Open::OpenString::padchar,i+Yn.min());
			IntSet dom_i(d);
			dom(*this, X[i], dom_i);
		}
		// Y = [0, 1, 2, 3, 4, <<< ...]
		for (int i = 0; i < Yn.min(); i++) {
			IntSet dom_i(i,i);
			dom(*this, Y[i], dom_i);
		}
		boilerplate();
  }
	bool passed(void) const {
		return Z[0].assigned() && Z[0].val()==5
			  && Z[1].assigned() && Z[1].val()==6
			  && Z[2].assigned() && Z[2].val()==7
			  && Z[3].min()==0 && Z[3].max()==8
				&& Z[4].min()==0 && Z[4].max()==9
				&& Z[5].min()==0 && Z[5].max()==2
				&& Z[6].min()==1 && Z[6].max()==3
				&& Z[7].min()==2 && Z[7].max()==4;
	}
};
class TestMandatoryY : public TestConcat {
public:
  TestMandatoryY(void) 
	: TestConcat("values in mandatory range of Y", 3, 5, 5, 15, 8, 15, 15) {
		// Z = [0,1,2,3,4,5,6,7,...
		for (int i = 0; i < Zn.min(); i++) {
			IntSet dom_i(i,i);
			dom(*this, Z[i], dom_i);
		}
		// ...{-1,8}, {-1,9}, ...]
		for (int i = Zn.min(); i < Zn.max(); i++) {
			IntArgs d(2,Open::OpenString::padchar,i);
			IntSet dom_i(d);
			dom(*this, Z[i], dom_i);
		}
		boilerplate();
  }
	bool passed(void) const {
		bool pass = true;
		for (int i = 0; i < 5; i++){
			if (Y[i].min() != i+Xn.min() || Y[i].max() != i+Xn.max())
				pass = false;
		}
		return pass;
	}
};
class TestMandatoryX : public TestConcat {
public:
  TestMandatoryX(void) 
	: TestConcat("disjoint in mandatory X ==> failure", 3, 5, 5, 10, 8, 10, 10) {
		IntArgs even(6,Open::OpenString::padchar,0,2,4,6,8);
		IntArgs  odd(6,Open::OpenString::padchar,1,3,5,7,9);
		IntSet evenSet(even);
		IntSet  oddSet(odd);
		dom(*this,X[1],evenSet);
		dom(*this,Z[1],oddSet);
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};

class TestMandatoryZFail : public TestConcat {
public:
  TestMandatoryZFail(void) 
	: TestConcat("disjoint in mandatory Z ==> FAIL", 3, 5, 5, 15, 8, 15, 15) {
		for (int i = 0; i < Xn.min(); i++) {
			IntSet dom_i(i+Yn.min(),i+Yn.min());
			dom(*this, X[i], dom_i);
		}
		for (int i = Xn.min(); i < Xn.max(); i++) {
			IntArgs d(2,Open::OpenString::padchar,i+Yn.min());
			IntSet dom_i(d);
			dom(*this, X[i], dom_i);
		}
		// Y = [0, 1, 2, 3, 4, <<< ...]
		for (int i = 0; i < Yn.min(); i++) {
			IntSet dom_i(i,i);
			dom(*this, Y[i], dom_i);
		}
		IntSet dom_i(4,4);
		dom(*this,Z[4],dom_i);
		boilerplate();
  }
	bool shouldPass(void) const {return false;}
};
class TestFullStretch : public TestConcat {
public:
  TestFullStretch(void) 
	: TestConcat("Zmax = Xmax+Ymax, but an optional Z is disjoint", 0, 5, 2, 10, 8, 15, 15) {
		IntSet dom1 = Open::OpenString::paddedIntSet(5,5);
		IntSet dom2 = Open::OpenString::paddedIntSet(6,6);
		dom(*this, Z[12], dom1);
		dom(*this, Y[7], dom2);
		boilerplate();
  }
	bool shouldPass(void) const {return true;}
	bool passed(void) const {
		return Zn.max() < 15;
	}
};
int passCount = 0;
int runCount = 0;
void runTest(TestConcat* test) {
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
	}
	std::cout << std::endl;
	test->print();
	std::cout << "\n=======\n";
	runCount++;
	if (pass) passCount++;
}

// main function
int main(int argc, char* argv[]) {
  // create model and search engine
  TestTooShort* m1 = new TestTooShort;
	runTest(m1);
  TestTooLong* m2 = new TestTooLong;
	runTest(m2);
  TestLengthInv1* m3 = new TestLengthInv1;
	runTest(m3);
  TestLengthInv2* m4 = new TestLengthInv2;
	runTest(m4);
  TestOptionalX* m5 = new TestOptionalX;
	runTest(m5);
  TestMandatoryZ* m6 = new TestMandatoryZ;
	runTest(m6);
  TestMandatoryY* m7 = new TestMandatoryY;
	runTest(m7);
  TestMandatoryX* m8 = new TestMandatoryX;
	runTest(m8);
  TestMandatoryZFail* m9 = new TestMandatoryZFail;
	runTest(m9);
	TestInvalidLengths* m10 = new TestInvalidLengths;
	runTest(m10);
	TestFullStretch* m11 = new TestFullStretch;
	runTest(m11);
	std::cout << passCount << " out of " << runCount << " tests passed." << std::endl;
  return 0;
}

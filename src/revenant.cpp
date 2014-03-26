#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "open-layered-graph.hh"

using namespace Gecode;

class Revenant : public Script {
private:
  IntVarArray A;
  IntVar N;
public:
  Revenant(const SizeOptions& opt) : 
    A(*this, opt.size()*2, 1, 3),
    N(*this, 1, opt.size()*2) {
      REG lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1);
      DFA d1(lang1);
      extensional(*this, A, d1, N);
      //extensional(*this, A, d1);
      REG lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size());
      DFA d2(lang2);
      extensional(*this, A, d2, N);
      //extensional(*this, A, d2);
      
      //branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN(), &filter);
      branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN());
      //branch(*this, N, INT_VAL_SPLIT_MIN());
    }
  bool bounded(IntVar x, int i) const {
    return i <= N.min();
  }
  static bool filter(const Space& home, IntVar x, int i) {
    return static_cast<const Revenant&>(home).bounded(x,i);
  }
  Revenant(bool share, Revenant& s) :
  Script(share,s) {
    A.update(*this, share, s.A);
    N.update(*this, share, s.N);
  }
  virtual Space* copy(bool share) {
    return new Revenant(share, *this);
  }
  virtual void print(std::ostream& os) const {
    os << A << std::endl;
  }
};

int main(int argc, char* argv[]) {
  SizeOptions opt("Revenant");
  opt.size(50);
  opt.solutions(0);
  opt.parse(argc,argv);
  Script::run<Revenant,DFS,SizeOptions>(opt);
  return 0;
}
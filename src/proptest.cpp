#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "open-layered-graph.hh"

using namespace Gecode;

class Revenant : public Space {
private:
  IntVarArray A;
  IntVar N;
public:
  enum {PROP_CLOSED, PROP_OPEN};
  Revenant(const SizeOptions& opt) : 
    A(*this, opt.size()*2, 1, 3),
    N(*this, 1, opt.size()*2) {
      REG lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1);
      DFA d1(lang1);
      REG lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size());
      DFA d2(lang2);
      
      switch(opt.propagation()) {
        case PROP_CLOSED:
          extensional(*this, A, d1);
          extensional(*this, A, d2);
          break;
        case PROP_OPEN:
          extensional(*this, A, d1, N);
          extensional(*this, A, d2, N);
          break;
        default: break;
      }
    }
  bool bounded(IntVar x, int i) const {
    return i <= N.min();
  }
  static bool filter(const Space& home, IntVar x, int i) {
    return static_cast<const Revenant&>(home).bounded(x,i);
  }
  Revenant(bool share, Revenant& s) :
  Space(share,s) {
    A.update(*this, share, s.A);
    N.update(*this, share, s.N);
  }
  virtual Space* copy(bool share) {
    return new Revenant(share, *this);
  }
  virtual void print(std::ostream& os) const {
    os << N << ":";
    // for (int i = 0; i < N.min(); i++)
    //   os << A[i];
    // os << std::endl;
    os << A << std::endl;
  }
};

int main(int argc, char* argv[]) {
  SizeOptions opt("Single propagation iteration");
  opt.size(50);
  opt.solutions(0);
  opt.propagation(Revenant::PROP_CLOSED,
                  "closed", "use fixed-length extensional propagator");
  opt.propagation(Revenant::PROP_OPEN,
                  "open", "use bounded-length extensional propagator");
  opt.propagation(Revenant::PROP_OPEN);
  opt.parse(argc,argv);
  
  Revenant* r = new Revenant(opt);
  std::cout << "before : ";
  r->print(std::cout);
  std::cout << std::endl << " after : ";
  SpaceStatus s = r->status();
  r->print(std::cout);
  if(s == SS_FAILED) {
    std::cout << "\tFAILED!!";
  }
  std::cout << std::endl;
  return 0;
}
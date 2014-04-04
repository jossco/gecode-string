#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "open-layered-graph.hh"

using namespace Gecode;

int wordlength;

class BoundedNone : public Brancher {
protected:
  ViewArray<Int::IntView> x;
  Int::IntView length;
  mutable int start;
  // choice definition
  class PosVal : public Choice {
  public:
    int pos; int val;
    PosVal(const BoundedNone& b, int p, int v)
      : Choice(b,2), pos(p), val(v) {}
    virtual size_t size(void) const {
      return sizeof(*this);
    }
    virtual void archive(Archive& e) const {
      Choice::archive(e);
      e << pos << val;
    }
  };
public:
  BoundedNone(Home home, ViewArray<Int::IntView>& x0, Int::IntView l)
    : Brancher(home), x(x0), length(l), start(0) {}
  static void post(Home home, ViewArray<Int::IntView>& x, Int::IntView length) {
    (void) new (home) BoundedNone(home,x,length);
  }
  virtual size_t dispose(Space& home) {
    (void) Brancher::dispose(home);
    return sizeof(*this);
  }
  BoundedNone(Space& home, bool share, BoundedNone& b)
    : Brancher(home,share,b), start(b.start) {
    x.update(home,share,b.x);
    length.update(home,share,b.length);
  }
  virtual Brancher* copy(Space& home, bool share) {
    return new (home) BoundedNone(home,share,*this);
  }
  // status
  virtual bool status(const Space& home) const {
    while (start < length.min()){
      if (!x[start].assigned()) {
        return true;
      }
      start++;
    }
    assert(start == length.min());
    if (!length.assigned())
      return true;
    return false;
  }
  // choice
  virtual Choice* choice(Space& home) {
    if(start >= length.min()){
      assert(!length.assigned());
      return new PosVal(*this,-1,length.med());
    }
    else {
      return new PosVal(*this,start,x[start].min());
    }
  }
  virtual Choice* choice(const Space&, Archive& e) {
    int pos, val;
    e >> pos >> val;
    return new PosVal(*this, pos, val);
  }
  // commit
  virtual ExecStatus commit(Space& home, 
                            const Choice& c,
                            unsigned int a) {
    const PosVal& pv = static_cast<const PosVal&>(c);
    int pos=pv.pos, val=pv.val;
    if (pos == -1) {
      //assert(start >= length.min());
      // branch on length
      if (a == 0)
        return me_failed(length.lq(home,val)) ? ES_FAILED : ES_OK;
      else
        return me_failed(length.gr(home,val)) ? ES_FAILED : ES_OK;
    } else {
      // branch on an array element
      if (a == 0)
        return me_failed(x[pos].eq(home,val)) ? ES_FAILED : ES_OK;
      else
        return me_failed(x[pos].nq(home,val)) ? ES_FAILED : ES_OK;
    }
  }
  // print
  virtual void print(const Space& home, const Choice& c,
                     unsigned int a,
                     std::ostream& o) const {
    const PosVal& pv = static_cast<const PosVal&>(c);
    int pos=pv.pos, val=pv.val;
    if (pos == -1) {
      if (a == 0)
        o << "length <= " << val;
      else 
        o << "length > " << val;
    } else {
      if (a == 0)
        o << "x[" << pos << "] = " << val;
      else
        o << "x[" << pos << "] != " << val;
    }
  }
};
void boundednone(Home home, const IntVarArgs& x, const IntVar length) {
  if (home.failed()) return;
  ViewArray<Int::IntView> y(home,x);
  Int::IntView l(length);
  BoundedNone::post(home,y,l);
}

class Revenant : public Script {
private:
  IntVarArray A;
  IntVar N;
  bool fixed;
public:
  enum {BRANCH_ALL_VALS, BRANCH_LEN_FILTERED, BRANCH_BOUNDED, SEARCH_STATUS, SEARCH_DFS, MODEL_OPEN, MODEL_PADDED, MODEL_FIXED};
  Revenant(const SizeOptions& opt) : 
    A(*this, wordlength, 0, 3),
    N(*this, 1, wordlength) {
      REG lang1, lang2;
      if (opt.model() != MODEL_PADDED) {
        lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1);
        lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size());
      } else {
        lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1) + REG(0)(0,wordlength);
        lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size()) + REG(0)(0,wordlength);
      }
      DFA d1(lang1);
      DFA d2(lang2);
    
      switch(opt.model()) {
        case MODEL_FIXED:
        case MODEL_PADDED:
          fixed = true;
          extensional(*this, A, d1);
          extensional(*this, A, d2);
          break;
        case MODEL_OPEN:
          fixed = false;
          extensional(*this, A, d1, N);
          extensional(*this, A, d2, N);
          break;
      }
      if (opt.model() == MODEL_OPEN) {
        switch(opt.branching()) {
          case BRANCH_ALL_VALS:
            branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, N, INT_VAL_SPLIT_MIN());
            break;
          case BRANCH_LEN_FILTERED:
            branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN(), &filter);
            branch(*this, N, INT_VAL_SPLIT_MIN());
            break;
          case BRANCH_BOUNDED:
            boundednone(*this, A, N);
            break;
        }
        
      } else {
        branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN());
      }
    }
  bool bounded(IntVar x, int i) const {
    return fixed ? i < wordlength : i < N.min();
  }
  static bool filter(const Space& home, IntVar x, int i) {
    return static_cast<const Revenant&>(home).bounded(x,i);
  }
  Revenant(bool share, Revenant& s) :
  Script(share,s), fixed(s.fixed) {
    A.update(*this, share, s.A);
    N.update(*this, share, s.N);
  }
  virtual Space* copy(bool share) {
    return new Revenant(share, *this);
  }
  virtual void print(std::ostream& os) const {
    if (fixed) {
      for (int i = 0; i < wordlength; i++)
        os << A[i];
    } else {
      for (int i = 0; i < N.min(); i++)
        os << A[i];
    } 
    os << std::endl;
  }
};

int main(int argc, char* argv[]) {
  SizeOptions opt("Revenant test");
  opt.size(50);
  opt.solutions(0);
  opt.branching(Revenant::BRANCH_ALL_VALS,
     "all", "branch on all characters in array");
  opt.branching(Revenant::BRANCH_LEN_FILTERED,
    "filter", "use filtering to branch on characters under min length");
  opt.branching(Revenant::BRANCH_BOUNDED,
    "filter", "use custome brancher to branch on characters under min length");
  opt.branching(Revenant::BRANCH_BOUNDED);
  opt.search(Revenant::SEARCH_DFS,"dfs", "dfs search");
  opt.search(Revenant::SEARCH_STATUS,"status", "no search, just filter one time");
  opt.search(Revenant::SEARCH_DFS);
  opt.model(Revenant::MODEL_OPEN,"open","bounded-length extensional propagation");
  opt.model(Revenant::MODEL_PADDED,"pad","fixed, maximal length string with padding characters");
  opt.model(Revenant::MODEL_FIXED,"fix","fixed length, solve for each possible length in sequence");
  opt.model(Revenant::MODEL_OPEN);
  opt.parse(argc,argv);
  if (opt.search() != Revenant::SEARCH_STATUS) {
    if (opt.model() == Revenant::MODEL_FIXED) {
      int count = 0;
      for (wordlength = 1; wordlength <= opt.size()*2; wordlength++ ) {
        Revenant* m = new Revenant(opt);
        DFS<Revenant> e(m);
        delete m;
        while (Revenant* s = e.next()) {
          s->print(std::cout); delete s; count++;
          if (opt.solutions() != 0 && count >= opt.solutions())
            break;
        }
        if (opt.solutions() != 0 && count >= opt.solutions())
          break;
      }
      std::cout << "Total Solutions: " << count << std::endl;
    } else {
      wordlength = opt.size()*2;
      Script::run<Revenant,DFS,SizeOptions>(opt);
    }
  }
  else {
    wordlength = opt.size()*2;
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
    delete r;
  }
  return 0;
}

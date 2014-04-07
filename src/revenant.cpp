#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "open-layered-graph.hh"
#include <fstream>
#include <iostream>
using namespace Gecode;

int wordlength;

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
      return new PosVal(*this,-1,length.min());
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
        return me_failed(length.eq(home,val)) ? ES_FAILED : ES_OK;
      else
        return me_failed(length.nq(home,val)) ? ES_FAILED : ES_OK;
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
        o << "length = " << val;
      else 
        o << "length != " << val;
    } else {
      if (a == 0)
        o << "x[" << pos << "] = " << val;
      else
        o << "x[" << pos << "] != " << val;
    }
  }
};
void boundednone(Home home, const IntVarArgs& x, IntVar length) {
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
  enum {BRANCH_A_N, BRANCH_N_A, BRANCH_FILTER, BRANCH_BOUND, SEARCH_STATUS, SEARCH_ITERATE, SEARCH_DFS, PROP_OPEN, PROP_PAD, PROP_CLOSED};
  Revenant(const SizeOptions& opt) : 
    A(*this, wordlength, 0, 3),
    N(*this, 1, wordlength) {
      REG lang1, lang2;
      if (opt.propagation() != PROP_PAD) {
        lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1);
        lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size());
      } else {
        lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(opt.size() + 1,opt.size() + 1) + REG(0)(0,wordlength);
        lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(opt.size(),opt.size()) + REG(0)(0,wordlength);
      }
      DFA d1(lang1);
      DFA d2(lang2);
    
      switch(opt.propagation()) {
        case PROP_CLOSED:
        case PROP_PAD:
          fixed = true;
          extensional(*this, A, d1);
          extensional(*this, A, d2);
          break;
        case PROP_OPEN:
          fixed = false;
          extensional(*this, A, d1, N);
          extensional(*this, A, d2, N);
          break;
      }
      switch(opt.branching()) {
        case BRANCH_N_A:
          if (opt.propagation() == PROP_OPEN) branch(*this, N, INT_VAL_MIN());
          branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN());
          break;
        case BRANCH_A_N:
          branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN());
          if (opt.propagation() == PROP_OPEN) branch(*this, N, INT_VAL_MIN());
          break;
        case BRANCH_FILTER:
          branch(*this, A, INT_VAR_NONE(), INT_VAL_MIN(), &filter);
          if (opt.propagation() == PROP_OPEN) branch(*this, N, INT_VAL_MIN());
          break;
        case BRANCH_BOUND:
          boundednone(*this, A, N);
          break;
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
      os << wordlength << " : [ ";
      for (int i = 0; i < wordlength; i++)
        os << A[i];
    } else {
      os << N << " : [ ";
      for (int i = 0; i < std::min(N.min(),A.size()); i++)
        os << A[i];
      if (!N.assigned()) {
        os << "{";
        for (int i = N.min(); i < std::min(A.size(),N.max()); i++)
          os << A[i];
        os << "}";
      }
    } 
    os << " ]" << std::endl;
  }
};

int main(int argc, char* argv[]) {
  
  SizeOptions opt("Revenant test");
                    // defaults:
  opt.size(5);      // a small instance
  opt.solutions(0); // all solutions
  opt.threads(1);   // sequential, deterministic search
  opt.c_d(1);       // no recomputation
    
  opt.branching(Revenant::BRANCH_A_N,    "an",      "branch array, then length");
  opt.branching(Revenant::BRANCH_N_A,    "na",      "branch length, then array");
  opt.branching(Revenant::BRANCH_FILTER, "filter",  "filter to branch on characters under min length");
  opt.branching(Revenant::BRANCH_BOUND,  "bound",   "custom brancher to branch on characters under min length");
  opt.branching(Revenant::BRANCH_BOUND);
  
  opt.search(Revenant::SEARCH_DFS,       "dfs",     "dfs search");
  opt.search(Revenant::SEARCH_ITERATE,   "iterate", "search each possible length in turn");
  opt.search(Revenant::SEARCH_STATUS,    "status",  "no search, just filter one time");
  opt.search(Revenant::SEARCH_DFS);
  
  opt.propagation(Revenant::PROP_OPEN,   "open",    "bounded-length extensional propagation");
  opt.propagation(Revenant::PROP_PAD,    "pad",     "fixed, maximal length string with padding characters");
  opt.propagation(Revenant::PROP_CLOSED, "closed",  "fixed-length extensional propagation");
  opt.propagation(Revenant::PROP_OPEN);
  
  opt.parse(argc,argv);
  switch(opt.search()) {
    case Revenant::SEARCH_STATUS:
      {
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
        break;
      }
    case Revenant::SEARCH_ITERATE:
      {
        std::ofstream sol_file;
        std::ostream& s_out = select_ostream(opt.out_file(), sol_file);
        Support::Timer t;
        t.start();
        Search::Statistics stat;
        Search::Options sopt;
        sopt.threads = opt.threads();
        sopt.c_d = opt.c_d();
        int solutions = 0;
        for (wordlength = 1; wordlength <= opt.size()*2; wordlength++ ) {
          Revenant* m = new Revenant(opt);
          DFS<Revenant> e(m,sopt);
          delete m;
          while (Revenant* s = e.next()) {
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
    case Revenant::SEARCH_DFS:
      {
        wordlength = opt.size()*2;
        Script::run<Revenant,DFS,SizeOptions>(opt);
        break;
      }
  }
  return 0;
}

#ifndef BOUNDED_NONE_HH
#define BOUNDED_NONE_HH


using namespace Gecode;

class BoundedNone : public Brancher {
protected:
  ViewArray<Int::IntView> x;
  Int::IntView length;
  mutable int start;
  // choice definition
  class PosVal : public Choice {
  public:
    int pos; int val;
    PosVal(const BoundedNone& b, int p, int v);
    virtual size_t size(void) const; 
    virtual void archive(Archive& e) const;
  };
public:
  BoundedNone(Home home, ViewArray<Int::IntView>& x0, Int::IntView l);
  static void post(Home home, ViewArray<Int::IntView>& x, Int::IntView length);
  virtual size_t dispose(Space& home);
  BoundedNone(Space& home, bool share, BoundedNone& b);
  virtual Brancher* copy(Space& home, bool share);
  // status
  virtual bool status(const Space& home) const;
  // choice
  virtual Choice* choice(Space& home);
  virtual Choice* choice(const Space&, Archive& e);
  // commit
  virtual ExecStatus commit(Space& home, 
                            const Choice& c,
                            unsigned int a);
  // print
  virtual void print(const Space& home, const Choice& c,
                     unsigned int a,
                     std::ostream& o) const;
};
void boundednone(Home home, const IntVarArgs& x, IntVar length);
#endif
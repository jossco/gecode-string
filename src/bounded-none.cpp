#include <gecode/int.hh>
#include "bounded-none.hh"
using namespace Gecode;

// choice definition
BoundedNone::PosVal::PosVal(const BoundedNone& b, int p, int v)
  : Choice(b,2), pos(p), val(v) {}
size_t BoundedNone::PosVal::size(void) const {
  return sizeof(*this);
}
void BoundedNone::PosVal::archive(Archive& e) const {
  Choice::archive(e);
  e << pos << val;
}

BoundedNone::BoundedNone(Home home, ViewArray<Int::IntView>& x0, Int::IntView l)
  : Brancher(home), x(x0), length(l), start(0) {}
void BoundedNone::post(Home home, ViewArray<Int::IntView>& x, Int::IntView length) {
  (void) new (home) BoundedNone(home,x,length);
}
size_t BoundedNone::dispose(Space& home) {
  (void) Brancher::dispose(home);
  return sizeof(*this);
}
BoundedNone::BoundedNone(Space& home, bool share, BoundedNone& b)
  : Brancher(home,share,b), start(b.start) {
  x.update(home,share,b.x);
  length.update(home,share,b.length);
}
Brancher* BoundedNone::copy(Space& home, bool share) {
  return new (home) BoundedNone(home,share,*this);
}
// status
bool BoundedNone::status(const Space& home) const {
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
Choice* BoundedNone::choice(Space& home) {
  if(start >= length.min()){
    assert(!length.assigned());
    return new PosVal(*this,-1,length.min());
  }
  else {
    return new PosVal(*this,start,x[start].min());
  }
}
Choice* BoundedNone::choice(const Space&, Archive& e) {
  int pos, val;
  e >> pos >> val;
  return new PosVal(*this, pos, val);
}
// commit
ExecStatus BoundedNone::commit(Space& home, 
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
void BoundedNone::print(const Space& home, const Choice& c,
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

void boundednone(Home home, const IntVarArgs& x, IntVar length) {
  if (home.failed()) return;
  ViewArray<Int::IntView> y(home,x);
  Int::IntView l(length);
  BoundedNone::post(home,y,l);
}
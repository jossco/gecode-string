/*
gecode-string:	bounded-length string constraints for Gecode
url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./src/open.cpp
version: 	0.2.1
date: 		Wed Dec  3 17:58:46 CET 2014
========
This file originally based on code generated by the indexical compiler,
written by JN Monette @ Uppsala University.
*/
#include "gecode/driver.hh"
#include "gecode/int.hh"
#include "gecode/iter.hh" 
#include "open.hh"
#include <iostream>

using namespace Gecode;

namespace Gecode { namespace Open {
	Iter::Ranges::Singleton Open::OpenString::PadRange(padchar,padchar);
	Iter::Ranges::Singleton Open::OpenString::Symbols(0,96);
// Union of integer range and the padding character
// (for initializing domains of "character" variables)
IntSet Open::OpenString::paddedIntSet(int n, int m) {
	int ranges[2][2] = {{padchar, padchar}, {n,m}};
	return IntSet(ranges,2);
}

void Open::OpenString::print(std::ostream& os, IntVarArray X, IntVar Xn) 
{
	os << Xn <<  ":" << "{";
	if (!Xn.assigned()) {
		for(int i=0; i<Xn.min()-1; i++)
		{
			os << i << ":" << X[i];
			os << ", ";
		}
		if (Xn.min() > 0)
			os << Xn.min()-1 << ":" << X[Xn.min()-1];
		os << "<<<";
		for(int i=Xn.min(); i<Xn.max()-1; i++)
		{
			os << i << ":" << X[i];
			os << ", ";
		}
		os << Xn.max()-1 << ":'" << X[Xn.max()-1] << ">>>";
		if (Xn.max() < X.size()){
			for(int i=Xn.max(); i<X.size()-1; i++)
			{
				os << i << ":" << X[i];
				os << ", ";
			}
			os << X.size()-1 << ":" << X[X.size()-1];
		}
	} else {
		for(int i=0; i<Xn.val()-1; i++)
		{
			os << i << ":" << X[i];
			os << ", ";
		}
		os << Xn.val()-1 << ":" << X[Xn.val()-1] << " | ";
		for(int i=Xn.val(); i<X.size()-1; i++)
		{
			os << i << ":" << X[i];
			os << ", ";
		}
		os << X.size()-1 << ":" << X[X.size()-1];
	}
	os << "}";
}
}}

void Open::OpenString::printAsString(std::ostream& os, IntVarArray X, IntVar Xn) {
	os << '"';
	if (Xn.assigned()) {
		for(int i = 0; i < Xn.val() && i < X.size(); i++) {
			if (X[i].assigned()) {
				os << (char)(X[i].val()+31);
			} else {
				os << X[i];
			}
		}
	}
	os << '"';
}
namespace Gecode { namespace Open { 
	Invariant::Invariant(Home home, ViewArray<Int::IntView> _X, Int::IntView _Xn)
		: Propagator(home), X(_X), Xn(_Xn){
		for (int i = 0; i < X.size(); i++){
			X[i].subscribe(home,*this,Int::PC_INT_DOM);
		}
		Xn.subscribe(home,*this,Int::PC_INT_BND);
	}
	Invariant::Invariant(Home home, bool share, Invariant& p)
		: Propagator(home, share, p){
		X.update(home, share, p.X);
		Xn.update(home,share,p.Xn);
	}
	size_t Invariant::dispose(Space& home){
		for (int i = 0; i < X.size(); i++){
			X[i].cancel(home,*this,Int::PC_INT_DOM);
		}
		Xn.cancel(home,*this,Int::PC_INT_BND);
		(void) Propagator::dispose(home);
		return sizeof(*this);
	}
	ExecStatus Invariant::post(Space& home, ViewArray<Int::IntView> X, Int::IntView Xn){
		//initial prop
	  GECODE_ME_CHECK(Xn.lq(home,X.size()));
		GECODE_ME_CHECK(Xn.gq(home,0));
		// adjust length bounds based on presence/absence of padding char
		for (int i = Xn.min(); i<Xn.max(); i++){
			if(X[i].assigned() && X[i].val() == Open::OpenString::padchar) {
				// X[i] is (the start of) the excluded region
				GECODE_ME_CHECK(Xn.lq(home,i));
				// BEWARE: this sets Xn.max=i, so the for loop ends here
			} else {
				if (Open::OpenString::padchar < X[i].min() || Open::OpenString::padchar > X[i].max()) {
					// X[i] in the mandatory region
					GECODE_ME_CHECK(Xn.gr(home,i));
				}
			}
		}
		// remove padding char from mandatory region
		for (int j=0; j<Xn.min(); j++) {
			GECODE_ME_CHECK(X[j].nq(home,Open::OpenString::padchar));
		}
		// excluded region contains only padding char
		for (int j=Xn.max(); j<X.size(); j++) {
			GECODE_ME_CHECK(X[j].eq(home,Open::OpenString::padchar));
		}
  
		(void) new (home) Invariant(home,X, Xn);
		return ES_OK;
	}
	ExecStatus Invariant::propagate(Space& home, const Gecode::ModEventDelta& med){
		// adjust length bounds based on presence/absence of padding char
		for (int i = Xn.min(); i<Xn.max(); i++){
			if(X[i].assigned() && X[i].val() == Open::OpenString::padchar) {
				// X[i] is (the start of) the excluded region
				GECODE_ME_CHECK(Xn.lq(home,i));
				// BEWARE: this sets Xn.max=i, so the for loop ends here
			} else {
				if (Open::OpenString::padchar < X[i].min() || Open::OpenString::padchar > X[i].max()) {
					// X[i] in the mandatory region
					GECODE_ME_CHECK(Xn.gr(home,i));
				}
			}
		}
		// remove padding char from mandatory region
		for (int j = 0; j < Xn.min(); j++) {
			GECODE_ME_CHECK(X[j].nq(home,Open::OpenString::padchar));
		}
		// excluded region contains only padding char
		for (int j = Xn.max(); j < X.size(); j++) {
			GECODE_ME_CHECK(X[j].eq(home,Open::OpenString::padchar));
		}
		
		//Subsumed?
		if(Xn.assigned()){
			return home.ES_SUBSUMED(*this);
		}
		return ES_FIX;
	}
}}

void open_invariant(Home home, IntVarArgs _X, IntVar _Xn) {
	if(home.failed())return;
	ViewArray<Int::IntView> X(home,_X);
	Int::IntView Xn(_Xn);
	GECODE_ES_FAIL(Open::Invariant::post(home,X, Xn));
}

namespace Gecode { namespace Open {

CharacterAt::CharacterAt(Home home, ViewArray<Int::IntView> _X, Int::IntView _Xn, Int::IntView _C, Int::IntView _Index)
: Propagator(home), X(_X),Xn(_Xn),C(_C),Index(_Index){
	 for(int _i_=0;_i_<=X.size()-1;_i_++){
	 	X[_i_].subscribe(home,*this,Int::PC_INT_BND);
	 } 
	Index.subscribe(home,*this,Int::PC_INT_BND);
	Xn.subscribe(home,*this,Int::PC_INT_BND);
	C.subscribe(home,*this,Int::PC_INT_BND);
}
CharacterAt::CharacterAt(Home home, bool share, CharacterAt& p):Propagator(home, share, p){
	X.update(home,share,p.X);
	Xn.update(home,share,p.Xn);
	C.update(home,share,p.C);
	Index.update(home,share,p.Index);
}
size_t CharacterAt::dispose(Space& home){
	 for(int _i_=0;_i_<=X.size()-1;_i_++){
	 	X[_i_].cancel(home,*this,Int::PC_INT_BND);
	 } 
	Index.cancel(home,*this,Int::PC_INT_BND);
	Xn.cancel(home,*this,Int::PC_INT_BND);
	C.cancel(home,*this,Int::PC_INT_BND);
	(void) Propagator::dispose(home);
	return sizeof(*this);
}

ExecStatus CharacterAt::post(Space& home, ViewArray<Int::IntView> X, Int::IntView Xn, Int::IntView C, Int::IntView Index){
	//initial prop
	bool nafp = false;

	GECODE_ME_CHECK_MODIFIED(nafp,Index.gq(home,0));
	GECODE_ME_CHECK_MODIFIED(nafp,Index.lq(home,(X.size() + -1)));

	(void) new (home) CharacterAt(home,X, Xn, C, Index);
	return ES_OK;
}
ExecStatus CharacterAt::propagate(Space& home, const Gecode::ModEventDelta& med){
	bool nafp = true;
	while(nafp){
		nafp = false;
		GECODE_ME_CHECK_MODIFIED(nafp,Index.lq(home,Xn.max()));
		int localvar3 = Index.min();
		GECODE_ME_CHECK_MODIFIED(nafp,Xn.gq(home,localvar3));

		Region r(home);
		Iter::Ranges::Empty empty_naryvar0;

		Iter::Ranges::NaryUnion naryvar0(r,empty_naryvar0); 
		 for(int ii2=localvar3;ii2<=Index.max();ii2++){
		 	Iter::Ranges::Singleton localvar40(X[ii2].min(),X[ii2].max()); 
		 	naryvar0 |= localvar40;
		 } 
		GECODE_ME_CHECK_MODIFIED(nafp,C.inter_r(home,naryvar0));
		bool localvar4 = Index.assigned();
		if(localvar4){
			GECODE_ME_CHECK_MODIFIED(nafp,X[Index.val()].gq(home,C.min()));
		} 
		if(localvar4){
			GECODE_ME_CHECK_MODIFIED(nafp,X[Index.val()].lq(home,C.max()));
		} 
		 for(int ii0=0;ii0<=(X.size() + -1);ii0++){
		 	if(((C.max() < X[ii0].min()) || (X[ii0].max() < C.min()))){
		 		GECODE_ME_CHECK_MODIFIED(nafp,Index.nq(home,ii0));
		 	} 
		 } 
		bool localvar19 = false;
		if((C.assigned() && (localvar4 && X[Index.val()].assigned()))){
			localvar19 = ((Index.max() <= Xn.min()) && ((0 <= Index.min()) && (C.val() == X[Index.val()].val())));
		}else{
			localvar19 = false;
		}
		if(localvar19){
			return home.ES_SUBSUMED(*this);
		} 
	}
	if(X.assigned()&&Xn.assigned()&&C.assigned()&&Index.assigned())return home.ES_SUBSUMED(*this);
	return ES_FIX;
}
}}

/*
 functions used to post the constraint.
 generated.
*/
void open_characterat(Home home, IntVarArgs _X, IntVar _Xn, IntVar _C, IntVar _Index){
	if(home.failed())return;
	ViewArray<Int::IntView> X(home,_X);
	Int::IntView Xn(_Xn);
	Int::IntView C(_C);
	Int::IntView Index(_Index);
	GECODE_ES_FAIL(Open::CharacterAt::post(home,X, Xn, C, Index));
}

 

namespace Gecode { namespace Open {
Concat::Concat(Home home, ViewArray<Int::IntView> _X, Int::IntView _Xn, ViewArray<Int::IntView> _Y, Int::IntView _Yn, ViewArray<Int::IntView> _Z, Int::IntView _Zn)
: Propagator(home), X(_X),Xn(_Xn),Y(_Y),Yn(_Yn),Z(_Z),Zn(_Zn){
	Zn.subscribe(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<std::min(Zn.max(),Z.size());_i_++){
		Z[_i_].subscribe(home,*this,Int::PC_INT_DOM);
	}
	Xn.subscribe(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<std::min(Xn.max(),X.size());_i_++){
		X[_i_].subscribe(home,*this,Int::PC_INT_DOM);
	}
	Yn.subscribe(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<std::min(Yn.max(),Y.size());_i_++){
		Y[_i_].subscribe(home,*this,Int::PC_INT_DOM);
	}
}
Concat::Concat(Home home, bool share, Concat& p):Propagator(home, share, p){
	X.update(home,share,p.X);
	Xn.update(home,share,p.Xn);
	Y.update(home,share,p.Y);
	Yn.update(home,share,p.Yn);
	Z.update(home,share,p.Z);
	Zn.update(home,share,p.Zn);
}
size_t Concat::dispose(Space& home){
	Zn.cancel(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<Z.size();_i_++){
		Z[_i_].cancel(home,*this,Int::PC_INT_BND);
	}
	Xn.cancel(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<X.size();_i_++){
		X[_i_].cancel(home,*this,Int::PC_INT_BND);
	}
	Yn.cancel(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<Y.size();_i_++){
		Y[_i_].cancel(home,*this,Int::PC_INT_BND);
	}
	(void) Propagator::dispose(home);
	return sizeof(*this);
}

ExecStatus Concat::post(Space& home, ViewArray<Int::IntView> X, Int::IntView Xn, ViewArray<Int::IntView> Y, Int::IntView Yn, ViewArray<Int::IntView> Z, Int::IntView Zn){
	//initial prop
	GECODE_ME_CHECK(Zn.lq(home,Z.size()));
  GECODE_ME_CHECK(Yn.lq(home,Y.size()));
  GECODE_ME_CHECK(Xn.lq(home,X.size()));
	GECODE_ME_CHECK(Zn.lq(home,Xn.max()+Yn.max()));
	GECODE_ME_CHECK(Zn.gq(home,Xn.min()+Yn.min()));
	GECODE_ME_CHECK(Xn.lq(home,Zn.max()-Yn.min()));
	GECODE_ME_CHECK(Xn.gq(home,Zn.min()-Yn.max()));
	GECODE_ME_CHECK(Yn.lq(home,Zn.max()-Xn.min()));
	GECODE_ME_CHECK(Yn.gq(home,Zn.min()-Xn.max()));
	(void) new (home) Concat(home,X, Xn, Y, Yn, Z, Zn);
	return ES_OK;
}
ExecStatus Concat::propagate(Space& home, const Gecode::ModEventDelta& med){
	// Prune lengths
	GECODE_ME_CHECK(Zn.gq(home,(Xn.min() + Yn.min())));
	GECODE_ME_CHECK(Zn.lq(home,(Xn.max() + Yn.max())));
	GECODE_ME_CHECK(Xn.gq(home,(Zn.min() - Yn.max())));
	GECODE_ME_CHECK(Xn.lq(home,(Zn.max() - Yn.min())));
	GECODE_ME_CHECK(Yn.gq(home,(Zn.min() - Xn.max())));
	GECODE_ME_CHECK(Yn.lq(home,(Zn.max() - Xn.min())));
	assert(Xn.max() + Yn.max() >= Zn.max());
	assert(Xn.min() + Yn.min() <= Zn.min());
	// X[0]..X[Xmin] = Z[0]..Z[Xmin]
	for(int i=0; i<Xn.min(); i++){
		Int::ViewRanges<Int::IntView> rz(Z[i]);
		GECODE_ME_CHECK(X[i].inter_r(home,rz));
		Int::ViewRanges<Int::IntView> rx(X[i]);
		GECODE_ME_CHECK(Z[i].narrow_r(home,rx));
	}
	// If the end of Z is only reachable by Y, might be able to prune Zn
	// If Y has any wiggle room, though, then there is no way to decide what
	// to prune, so this is a pretty limited case:
	// if (Xn.max() + Yn.max() == Zn.max()) {
	// 	bool done = false;
	// 	for(int j = Xn.max(); j < Zn.max() && !done ; j++) {
	// 		// this region can only align with the end of Y
	// 		Int::ViewRanges<Int::IntView> rz(Z[j]);
	// 		Iter::Ranges::SubRange<Int::ViewRanges<Int::IntView>> rz2(rz, Open::OpenString::padchar, Open::OpenString::padchar);
	// 		Int::ViewRanges<Int::IntView> ry(Y[j - (Zn.max() - Yn.max())]);
	// 		if (Iter::Ranges::disjoint(rz2,ry)){
	// 			int zMax = Zn.max();
	// 			GECODE_ME_CHECK(Zn.le(home,Zmax));
	// 			done = true;
	// 		}
	// 	}
	// }
	// GECODE_ME_CHECK(Xn.lq(home,Zn.max()-Yn.min()));
	// X[Xmin]..X[min(Xmax,Zmin)]
	// if X[i] and Z[i] are disjoint, then Xmax <= i
	int r2max = std::min(Xn.max(),Zn.min());
	for(int i=Xn.min(); i < r2max; i++) {
		Int::ViewRanges<Int::IntView> rz(Z[i]);
		Int::ViewRanges<Int::IntView> rx(X[i]);
		Iter::Ranges::Inter<Int::ViewRanges<Int::IntView>,Int::ViewRanges<Int::IntView>> ri(rz,rx);
		if (Iter::Ranges::equal(ri,Open::OpenString::PadRange)){
			GECODE_ME_CHECK(Xn.lq(home,i));
			GECODE_ME_CHECK(Zn.lq(home,i+Yn.max()));
			r2max = Xn.max(); // must always be lower than Zn.min()
		}
	}
	// Z[Xmin]..Z[Zmin-1] 
	// Z[i] matches Y[0]..Y[j] (or X[i] iff i < Xmax)
	Iter::Ranges::NaryUnion* naryvar1= new Iter::Ranges::NaryUnion[Yn.min()];
	Iter::Ranges::Empty empty_naryvar1;
	for(int i = 0; i < Yn.min(); i++) {
		Region r(home);
		if (i >= Xn.max()-Xn.min()) {
			// Z[i + Xmin] is beyond X[Xmax]
			naryvar1[i] = Iter::Ranges::NaryUnion(r,empty_naryvar1);
		} else {
			// Z[i + Xmin] might align with X[i + Xmin]
			// Note that PAD in X[i + Xmin], but won't affect anything:
			// already have PAD notin Z[i + Xmin], since i + Xmin < Zmin.
			Int::ViewRanges<Int::IntView> rx(X[i+Xn.min()]);
			naryvar1[i] = Iter::Ranges::NaryUnion(r,rx);
		}
		// Union of every Y[j] that could align with Z[i + Xmin]
		// max() enforces lb of sliding window of width Xn.size()
		for(int j = std::max(0,i-(Xn.max()-Xn.min())); j <= i; j++){
			Int::ViewRanges<Int::IntView> ry(Y[j]);
			naryvar1[i] |= ry;
		}
		GECODE_ME_CHECK(Z[i + Xn.min()].inter_r(home,naryvar1[i]));
	}
	// Y[0]..Y[Ymin-1]
	// Y[j] matches Z[Xmin+j]..Z[Xmax+j]
	Iter::Ranges::NaryUnion* naryvar2= new Iter::Ranges::NaryUnion[Yn.min()];
	for(int j = 0; j < Yn.min(); j++) {
		Region r(home);
		naryvar2[j] = Iter::Ranges::NaryUnion(r,empty_naryvar1);
		for (int i = j + Xn.min(); i <= j + Xn.max(); i++) {
			Int::ViewRanges<Int::IntView> rz(Z[i]);
			naryvar2[j] |= rz;
		}
		GECODE_ME_CHECK(Y[j].inter_r(home, naryvar2[j]));
	}
	//TODO: if Xn.assigned(), should be able to rewrite as:
	//		Equal(X, Z[0:Xmax]) and Equal(Y, Z[Xmax:Zmax])
 	if(X.assigned()&&Xn.assigned()&&Y.assigned()&&Yn.assigned()&&Z.assigned()&&Zn.assigned())return home.ES_SUBSUMED(*this);
	return ES_FIX;
}
}}
//
// /*
//  functions used to post the constraint.
//  generated.
// */
void open_concat(Home home, IntVarArgs _X, IntVar _Xn, IntVarArgs _Y, IntVar _Yn, IntVarArgs _Z, IntVar _Zn){
	if(home.failed())return;
	Int::IntView Xn(_Xn), Yn(_Yn), Zn(_Zn);
  ViewArray<Int::IntView> X(home,_X);
  ViewArray<Int::IntView> Y(home,_Y);
  ViewArray<Int::IntView> Z(home,_Z);
	// If Zn is same variable as Xn or Yn, post simpler equality instead
	// if (same(Xn,Zn)) {
// 		GECODE_ME_FAIL(Yn.eq(home,0));
// 		GECODE_ES_FAIL(Open::Equal::post(home,X,Xn,Z,Zn));
// 	} else if (same(Yn,Zn)) {
// 		GECODE_ME_FAIL(Xn.eq(home,0));
// 		GECODE_ES_FAIL(Open::Equal::post(home,Y,Yn,Z,Zn));
// 	} else {
		GECODE_ES_FAIL(Open::Concat::post(home,X, Xn, Y, Yn, Z, Zn));
	// }
	// TODO: Worth checking if Xn.assigned()? Could post a couple of Equal()s.
}

namespace Gecode { namespace Open {
Equal::Equal(Home home, ViewArray<Int::IntView> _X, Int::IntView _Xn, ViewArray<Int::IntView> _Y, Int::IntView _Yn)
: Propagator(home), X(_X),Xn(_Xn),Y(_Y),Yn(_Yn){
	Xn.subscribe(home,*this,Int::PC_INT_BND);
	Yn.subscribe(home,*this,Int::PC_INT_BND);
	for(int _i_=0;_i_<X.size();_i_++){
		X[_i_].subscribe(home,*this,Int::PC_INT_DOM);
	}
	for(int _i_=0;_i_<Y.size();_i_++){
		Y[_i_].subscribe(home,*this,Int::PC_INT_DOM);
	} 
}
Equal::Equal(Home home, bool share, Equal& p):Propagator(home, share, p){
	X.update(home,share,p.X);
	Xn.update(home,share,p.Xn);
	Y.update(home,share,p.Y);
	Yn.update(home,share,p.Yn);
}
size_t Equal::dispose(Space& home){
	Xn.cancel(home,*this,Int::PC_INT_BND);
	Yn.cancel(home,*this,Int::PC_INT_BND);
	 for(int _i_=0;_i_<X.size();_i_++){
	 	X[_i_].cancel(home,*this,Int::PC_INT_DOM);
	}
	for(int _i_=0;_i_<Y.size();_i_++){
		Y[_i_].cancel(home,*this,Int::PC_INT_DOM);
	 } 
	(void) Propagator::dispose(home);
	return sizeof(*this);
}

ExecStatus Equal::post(Space& home, ViewArray<Int::IntView> X, Int::IntView Xn, ViewArray<Int::IntView> Y, Int::IntView Yn){
	//initial prop
	int maxLength = std::min(X.size(), Y.size());
  GECODE_ME_CHECK(Yn.lq(home,maxLength));
  if(!same(Yn,Xn)){
		GECODE_ME_CHECK(Xn.lq(home,maxLength));
    GECODE_ME_CHECK(Yn.gq(home,Xn.min()));
    GECODE_ME_CHECK(Xn.gq(home,Yn.min()));
  }
	for(int i=0; i < Xn.min(); i++){
		assert(i <= Y.size());
		Int::ViewRanges<Int::IntView> ry(Y[i]);
		GECODE_ME_CHECK(X[i].inter_r(home,ry));
		assert(i <= X.size());
		Int::ViewRanges<Int::IntView> rx(X[i]);
		GECODE_ME_CHECK(Y[i].narrow_r(home,rx));
	}
  if(!same(Yn,Xn)){
    GECODE_ME_CHECK(Yn.lq(home,Xn.max()));
    GECODE_ME_CHECK(Xn.lq(home,Yn.max()));
  }
	for(int i=Xn.min(); i < Xn.max(); i++) {
		assert(i <= Y.size());
		Int::ViewRanges<Int::IntView> ry(Y[i]);
		assert(i <= X.size());
		Int::ViewRanges<Int::IntView> rx(X[i]);
		Iter::Ranges::Inter<Int::ViewRanges<Int::IntView>,Int::ViewRanges<Int::IntView>> ri(ry,rx);
		if (Iter::Ranges::equal(ri,Open::OpenString::PadRange)){
			GECODE_ME_CHECK(Xn.lq(home,i));
			GECODE_ME_CHECK(Yn.lq(home,i));
		}
	}
	(void) new (home) Equal(home,X, Xn, Y, Yn);
	return ES_OK;
}
ExecStatus Equal::propagate(Space& home, const Gecode::ModEventDelta& med){
	int YnMin = Yn.min();
	GECODE_ME_CHECK(Xn.gq(home,YnMin));
	GECODE_ME_CHECK(Xn.lq(home,Yn.max()));
	int XnMin = Xn.min();
	GECODE_ME_CHECK(Yn.gq(home,XnMin));
	GECODE_ME_CHECK(Yn.lq(home,Xn.max()));

	for(int i=0;i<Xn.min();i++){
		assert(i <= Y.size());
		Int::ViewRanges<Int::IntView> ry(Y[i]);
		GECODE_ME_CHECK(X[i].inter_r(home,ry));
		assert(i <= X.size());
		Int::ViewRanges<Int::IntView> rx(X[i]);
		GECODE_ME_CHECK(Y[i].narrow_r(home,rx));
	} 
	for(int i=Xn.min(); i < Xn.max(); i++) {
		assert(i <= Y.size());
		Int::ViewRanges<Int::IntView> ry(Y[i]);
		assert(i <= X.size());
		Int::ViewRanges<Int::IntView> rx(X[i]);
		Iter::Ranges::Inter<Int::ViewRanges<Int::IntView>,Int::ViewRanges<Int::IntView>> ri(ry,rx);
		if (Iter::Ranges::equal(ri,Open::OpenString::PadRange)){
			GECODE_ME_CHECK(Xn.lq(home,i));
			GECODE_ME_CHECK(Yn.lq(home,i));
		}
	}
	// If we wait for the invariant propagator to assign the excluded vars,
	// we will miss subsumption now. So we go ahead and check just the 
	// mandatory and optional regions.
	bool Xassigned = true;
	bool Yassigned = true;
	for(int i = 0; i< Xn.max(); i++){
		assert(i <= Y.size());
		assert(i <= X.size());
		Xassigned = (Xassigned && X[i].assigned());
		Yassigned = (Yassigned && Y[i].assigned());
	}
	
	if(Xassigned && Xn.assigned() && Yassigned && Yn.assigned()){
		if (Xn.assigned() && Yn.assigned()){
			return home.ES_SUBSUMED(*this);
		} else {
			//GECODE_REWRITE(*this, (Int::Rel::EqBnd<Int::IntView,Int::IntView>::post(home(*this),Xn,Yn)));
		}
	}
	return ES_FIX;
}
}}
/*
 functions used to post the constraint.
 generated.
*/
void open_equal(Home home, IntVarArgs _X, IntVar _Xn, IntVarArgs _Y, IntVar _Yn){
	if(home.failed())return;
  ViewArray<Int::IntView> X(home,_X);
  ViewArray<Int::IntView> Y(home,_Y);
	GECODE_ES_FAIL(Open::Equal::post(home,X, _Xn, Y, _Yn));
}

namespace Gecode { namespace Open {
Substring::Substring(Home home, ViewArray<Int::IntView> _X, Int::IntView _Xn, ViewArray<Int::IntView> _Y, Int::IntView _Yn, Int::IntView _Index)
: Propagator(home), X(_X),Xn(_Xn),Y(_Y),Yn(_Yn),Index(_Index){
	 for(int _i_=0;_i_<=X.size()-1;_i_++){
	 	X[_i_].subscribe(home,*this,Int::PC_INT_BND);
	 } 
	Index.subscribe(home,*this,Int::PC_INT_BND);
	Yn.subscribe(home,*this,Int::PC_INT_BND);
	Xn.subscribe(home,*this,Int::PC_INT_BND);
	 for(int _i_=0;_i_<=Y.size()-1;_i_++){
	 	Y[_i_].subscribe(home,*this,Int::PC_INT_BND);
	 } 
}
Substring::Substring(Home home, bool share, Substring& p):Propagator(home, share, p){
	X.update(home,share,p.X);
	Xn.update(home,share,p.Xn);
	Y.update(home,share,p.Y);
	Yn.update(home,share,p.Yn);
	Index.update(home,share,p.Index);
}
size_t Substring::dispose(Space& home){
	 for(int _i_=0;_i_<=X.size()-1;_i_++){
	 	X[_i_].cancel(home,*this,Int::PC_INT_BND);
	 } 
	Index.cancel(home,*this,Int::PC_INT_BND);
	Yn.cancel(home,*this,Int::PC_INT_BND);
	Xn.cancel(home,*this,Int::PC_INT_BND);
	 for(int _i_=0;_i_<=Y.size()-1;_i_++){
	 	Y[_i_].cancel(home,*this,Int::PC_INT_BND);
	 } 
	(void) Propagator::dispose(home);
	return sizeof(*this);
}

ExecStatus Substring::post(Space& home, ViewArray<Int::IntView> X, Int::IntView Xn, ViewArray<Int::IntView> Y, Int::IntView Yn, Int::IntView Index){
	//initial prop
	(void) new (home) Substring(home,X, Xn, Y, Yn, Index);
	return ES_OK;
}
ExecStatus Substring::propagate(Space& home, const Gecode::ModEventDelta& med){
	bool nafp = true;
	while(nafp){
		nafp = false;
		int localvar742 = Xn.max();
		int localvar743 = Index.min();
		int localvar744 = -localvar743;
		GECODE_ME_CHECK_MODIFIED(nafp,Yn.lq(home,(localvar742 + localvar744)));
		int localvar747 = Yn.min();
		GECODE_ME_CHECK_MODIFIED(nafp,Index.lq(home,(localvar742 + -localvar747)));
		GECODE_ME_CHECK_MODIFIED(nafp,Xn.gq(home,(localvar747 + localvar743)));
		int localvar753 = Index.max();
		int localvar754 = std::max(localvar753,0);
		int localvar755 = Xn.min();
		int localvar756 = (localvar755 + -1);
		int localvar759 = (localvar747 + -1);
		int localvar761 = std::min(localvar756,(localvar743 + localvar759));
		if((localvar754 <= localvar761)){
			int localvar865 = -localvar753;
			int localvar866 = std::min(localvar865,0);
			int localvar874 = (localvar866 + localvar761);
			Iter::Ranges::NaryUnion* naryvar13 = new Iter::Ranges::NaryUnion[localvar874+1];
			 for(int i=0;i<=localvar874;i++){
			 	Region r(home);
			 	Iter::Ranges::Empty empty_naryvar13_i_;

			 	naryvar13[i] = Iter::Ranges::NaryUnion(r,empty_naryvar13_i_); 
			 	 for(int ii22=(i + (localvar754 + localvar865));ii22<=(i + (localvar754 + localvar744));ii22++){
			 	 	Iter::Ranges::Singleton localvar968(Y[ii22].min(),Y[ii22].max()); 
			 	 	naryvar13[i] |= localvar968;
			 	 } 
			 } 
			 for(int i=localvar754;i<=localvar761;i++){
			 	GECODE_ME_CHECK_MODIFIED(nafp,X[i].inter_r(home,naryvar13[(i + localvar866)]));
			 	int localvar776 = Y.size();
			 	GECODE_ME_CHECK_MODIFIED(nafp,Index.gq(home,(-localvar776 + (i + 1))));
			 	GECODE_ME_CHECK_MODIFIED(nafp,Index.lq(home,i));
			 	 for(int ii15=0;ii15<=(localvar776 + -1);ii15++){
			 	 	if(((X[i].max() < Y[ii15].min()) || (Y[ii15].max() < X[i].min()))){
			 	 		GECODE_ME_CHECK_MODIFIED(nafp,Index.nq(home,(-ii15 + i)));
			 	 	} 
			 	 } 
			 } 
		} 
		int localvar793 = std::max(localvar744,0);
		int localvar797 = -localvar753;
		int localvar801 = std::min(localvar759,(localvar797 + localvar756));
		if((localvar793 <= localvar801)){
			int localvar887 = std::min(localvar743,0);
			int localvar896 = (localvar887 + localvar801);
			Iter::Ranges::NaryUnion* naryvar14 = new Iter::Ranges::NaryUnion[localvar896+1];
			 for(int j=0;j<=localvar896;j++){
			 	Region r(home);
			 	Iter::Ranges::Empty empty_naryvar14_j_;

			 	naryvar14[j] = Iter::Ranges::NaryUnion(r,empty_naryvar14_j_); 
			 	 for(int ii8=(j + (localvar793 + localvar743));ii8<=(j + (localvar793 + localvar753));ii8++){
			 	 	Iter::Ranges::Singleton localvar983(X[ii8].min(),X[ii8].max()); 
			 	 	naryvar14[j] |= localvar983;
			 	 } 
			 } 
			 for(int j=localvar793;j<=localvar801;j++){
			 	int localvar814 = -j;
			 	GECODE_ME_CHECK_MODIFIED(nafp,Index.gq(home,localvar814));
			 	int localvar815 = X.size();
			 	GECODE_ME_CHECK_MODIFIED(nafp,Index.lq(home,(localvar815 + (localvar814 + -1))));
			 	 for(int ii9=0;ii9<=(localvar815 + -1);ii9++){
			 	 	if(((Y[j].max() < X[ii9].min()) || (X[ii9].max() < Y[j].min()))){
			 	 		GECODE_ME_CHECK_MODIFIED(nafp,Index.nq(home,(ii9 + localvar814)));
			 	 	} 
			 	 } 
			 	GECODE_ME_CHECK_MODIFIED(nafp,Y[j].inter_r(home,naryvar14[(j + localvar887)]));
			 } 
		} 
		bool naryvar15 = true;
		int localvar909 = std::max(localvar743,0);
		int localvar911 = (localvar742 + -1);
		int localvar913 = Yn.max();
		int localvar914 = (localvar913 + -1);
		int localvar916 = std::min(localvar911,(localvar753 + localvar914));
		 for(int i=localvar909;i<=localvar916;i++){
		 	bool localvar917 = false;
		 	if((X[i].assigned() && (Index.assigned() && Y[(i + -Index.val())].assigned()))){
		 		localvar917 = (X[i].val() == Y[(i + -Index.val())].val());
		 	}else{
		 		localvar917 = false;
		 	}
		 	naryvar15 = (naryvar15 && localvar917);
		 } 
		bool naryvar16 = true;
		int localvar920 = std::max(localvar797,0);
		int localvar928 = std::min(localvar914,(localvar744 + localvar911));
		 for(int j=localvar920;j<=localvar928;j++){
		 	bool localvar929 = false;
		 	if((Index.assigned() && (X[(j + Index.val())].assigned() && Y[j].assigned()))){
		 		localvar929 = (X[(j + Index.val())].val() == Y[j].val());
		 	}else{
		 		localvar929 = false;
		 	}
		 	naryvar16 = (naryvar16 && localvar929);
		 } 
		if((((localvar913 + localvar753) <= localvar755) && (((localvar916 < localvar909) || naryvar15) && ((localvar928 < localvar920) || naryvar16)))){
			return home.ES_SUBSUMED(*this);
		} 
	}
	if(X.assigned()&&Xn.assigned()&&Y.assigned()&&Yn.assigned()&&Index.assigned())return home.ES_SUBSUMED(*this);
	return ES_FIX;
}
}}

/*
 functions used to post the constraint.
 generated.
*/
void open_substring(Home home, IntVarArgs _X, IntVar _Xn, IntVarArgs _Y, IntVar _Yn, IntVar _Index){
	if(home.failed())return;
	ViewArray<Int::IntView> X(home,_X);
	Int::IntView Xn(_Xn);
	ViewArray<Int::IntView> Y(home,_Y);
	Int::IntView Yn(_Yn);
	Int::IntView Index(_Index);
	GECODE_ES_FAIL(Open::Substring::post(home,X, Xn, Y, Yn, Index));
}

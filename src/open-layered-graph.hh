#ifndef __GECODE_OPEN_LG_HH__
#define __GECODE_OPEN_LG_HH__

#include <gecode/int.hh>

/* from
*	gecode/int.hh:
*/
namespace Gecode {
	/**
	* \brief Post open domain consistent propagator for extensional constraint described by a DFA
	*
	* The first \a n elements of \a x must be a word of the language described by
	* the DFA \a d.
	*
	* Throws an exception of type Int::ArgumentSame, if \a x contains
	* the same unassigned variable multiply. If shared occurences of variables
	* are required, unshare should be used.
	*/
	GECODE_INT_EXPORT void
		extensional(Home home, const IntVarArgs& x, DFA d, const IntVar n);
}

/*	from
*	gecode/int/extensional.hh:
*/
namespace Gecode { namespace Int { namespace Extensional {

   /**
    * \brief Domain consistent layered graph (regular) propagator
    *
    * The algorithm for the regular propagator is based on:
    *   Gilles Pesant, A Regular Language Membership Constraint
    *   for Finite Sequences of Variables, CP 2004.
    *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
    *
    * The propagator is not capable of dealing with multiple occurences
    * of the same view.
    *
    * Requires \code #include <gecode/int/extensional.hh> \endcode
    * \ingroup FuncIntProp
    */
   template<class View, class Val, class Degree, class StateIdx>
   class OpenLayeredGraph : public Propagator {
   protected:
     /// States are described by number of incoming and outgoing edges
     class State {
     public:
       Degree i_deg; ///< The in-degree (number of incoming edges)
       Degree o_deg; ///< The out-degree (number of outgoing edges)
       /// Initialize with zeroes
       void init(void);
     };
     /// %Edge defined by in-state and out-state
     class Edge {
     public:
       StateIdx i_state; ///< Number of in-state
       StateIdx o_state; ///< Number of out-state
     };
     /// %Support information for a value
     class Support {
     public:
       Val val; ///< Supported value
       Degree n_edges; ///< Number of supporting edges
       Edge* edges; ///< Supporting edges in layered graph
     };
     /// Type for support size
     typedef typename Gecode::Support::IntTypeTraits<Val>::utype ValSize;
     /// %Layer for a view in the layered graph
     class Layer {
     public:
       View x; ///< Integer view
       StateIdx n_states; ///< Number of states used by outgoing edges
       ValSize size; ///< Number of supported values
       State* states; ///< States used by outgoing edges
       Support* support; ///< Supported values
     };
     /// Iterator for telling variable domains by scanning support
     class LayerValues {
     private:
       const Support* s1; ///< Current support
       const Support* s2; ///< End of support
     public:
       /// Default constructor
       LayerValues(void);
       /// Initialize for support of layer \a l
       LayerValues(const Layer& l);
       /// Initialize for support of layer \a l
       void init(const Layer& l);
       /// Test whether more values supported
       bool operator ()(void) const;
       /// Move to next supported value
       void operator ++(void);
       /// Return supported value
       int val(void) const;
     };
     /// %Advisors for views (by position in array)
     class Index : public Advisor {
     public:
       /// The position of the view in the view array
       int i;
       /// Create index advisor
       Index(Space& home, Propagator& p, Council<Index>& c, int i);
       /// Clone index advisor \a a
       Index(Space& home, bool share, Index& a);
     };
     /// Range approximation of which positions have changed
     class IndexRange {
     private:
       int _fst; ///< First index
       int _lst; ///< Last index
     public:
       /// Initialize range as empty
       IndexRange(void);
       /// Reset range to be empty
       void reset(void);
       /// Add index \a i to range
       void add(int i);
       /// Add index range \a ir to range
       void add(const IndexRange& ir);
       /// Shift index range by \a n elements to the left
       void lshift(int n);
       /// Test whether range is empty
       bool empty(void) const;
       /// Return first position
       int fst(void) const;
       /// Return last position
       int lst(void) const;
     };
     /// The original DFA
     DFA dfa;
     /// The advisor council
     Council<Index> c;
     /// Number of layers (and views)
     int n;
     /// The bounded length
     Int::IntView length;
     /// The layers of the graph
     Layer* layers;
     /// Maximal number of states per layer
     StateIdx max_states;
     /// Total number of states
     unsigned int n_states;
     /// Total number of edges
     unsigned int n_edges;
     /// Index range with in-degree modifications
     IndexRange i_ch;
     /// Index range with out-degree modifications
     IndexRange o_ch;
     /// Index range for any change (for compression)
     IndexRange a_ch;
     /// Return in state for layer \a i and state index \a is
     State& i_state(int i, StateIdx is);
     /// Return in state for layer \a i and in state of edge \a e
     State& i_state(int i, const Edge& e);
     /// Decrement out degree for in state of edge \a e for layer \a i
     bool i_dec(int i, const Edge& e);
     /// Return out state for layer \a i and state index \a os
     State& o_state(int i, StateIdx os);
     /// Return state for layer \a i and out state of edge \a e
     State& o_state(int i, const Edge& e);
     /// Decrement in degree for out state of edge \a e for layer \a i
     bool o_dec(int i, const Edge& e);
     /// Perform consistency check on data structures
     void audit(void);
     /// Initialize layered graph
     template<class Var>
     ExecStatus initialize(Space& home, 
                           const VarArgArray<Var>& x);
     /// Constructor for cloning \a p
     OpenLayeredGraph(Space& home, bool share,
                  OpenLayeredGraph<View,Val,Degree,StateIdx>& p);
   public:
     /// Constructor for posting
     template<class Var>
     OpenLayeredGraph(Home home, 
                  const VarArgArray<Var>& x, const DFA& dfa, Int::IntView length);
     /// Copy propagator during cloning
     virtual Actor* copy(Space& home, bool share);
     /// Cost function (defined as high linear)
     virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
     /// Give advice to propagator
     virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
     /// Perform propagation
     virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
     /// Delete propagator and return its size
     virtual size_t dispose(Space& home);
     /// Post propagator on views \a x and DFA \a dfa
     template<class Var>
     static ExecStatus post(Home home, 
                            const VarArgArray<Var>& x, const DFA& dfa, Int::IntView length);
   };

   /// Select small types for the layered graph propagator
   template<class Var>
   ExecStatus post_lgp(Home home, 
                       const VarArgArray<Var>& x, const DFA& dfa, Int::IntView length);

}}}


#endif
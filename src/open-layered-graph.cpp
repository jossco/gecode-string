#include <open-layered-graph.hh>
#include <climits>
#include <algorithm>
#include <fst/fstlib.h>

using namespace fst;

/*	from
*	gecode/int/extensional.cpp
*/
namespace Gecode {

	void
	extensional(Home home, const IntVarArgs& x, DFA dfa, IntVar length) {
		using namespace Int;
		if (x.same(home))
			throw ArgumentSame("Int::extensional");
		if (home.failed()) return;
    if (length.min() > x.size()) {
      throw TooFewArguments("Int::extensional");
    }
    if (length.assigned()) {
      if (x.size() == length.val())
        GECODE_ES_FAIL(Extensional::post_lgp(home,x,dfa));
      else {
        IntVarArgs _x;
        for (int i = 0; i < length.val(); i++){
          _x << x[i];
        }
        GECODE_ES_FAIL(Extensional::post_lgp(home,_x,dfa));
      }
    }
    else
		  GECODE_ES_FAIL(Extensional::post_lgp(home,x,dfa,length));
	}
}

namespace Gecode { namespace Int { namespace Extensional {
  /*
    TODO: eliminate OVarTraits
    OVarTraits is just a copy of Extensional::VarTraits from layered-graph.hpp.
    Unfortunately, VarTraits is NOT defined in extensional.hh, so it isn't visible
    from here (as long as we're building this outside of gecode, anyway.)
  
    If this gets rolled into gecode/int/extensional, these declarations should be removed.
  */
  
  
  /**
   * \brief Traits class for variables
   *
   * Each variable must specialize this traits class and add a \code
   * typedef \endcode for the view \a View corresponding to this variable.
   */
  template<class Var>
  class OVarTraits {};
  
  /**
   * \brief Traits class for variables
   *
   * This class specializes the VarTraits for integer variables.
   */
  template<>
  class OVarTraits<IntVar> {
  public:
    /// The variable type of an IntView
    typedef Int::IntView View;
  };
  
  /**
   * \brief Traits class for variables
   *
   * This class specializes the OVarTraits for Boolean variables.
   */
  template<>
  class OVarTraits<BoolVar> {
  public:
    /// The variable type of an IntView
    typedef Int::BoolView View;
  };

  template<class View>
  class ViewTraits {};
  
  template<>
  class ViewTraits<Int::IntView> {
  public:
    typedef IntVar Var;
  };
  
  template<>
  class ViewTraits<Int::BoolView> {
  public:
    typedef BoolVar Var;
  };

  /*
   * States
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::State::init(void) {
    i_deg=o_deg=0; 
  }

  
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename OpenLayeredGraph<View,Val,Degree,StateIdx>::State& 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::i_state(int i, StateIdx is) {
    return layers[i].states[is];
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename OpenLayeredGraph<View,Val,Degree,StateIdx>::State& 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::i_state
  (int i, const typename OpenLayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return i_state(i,e.i_state);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::i_dec
  (int i, const typename OpenLayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return --i_state(i,e).o_deg == 0;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename OpenLayeredGraph<View,Val,Degree,StateIdx>::State& 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::o_state(int i, StateIdx os) {
    return layers[i+1].states[os];
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename OpenLayeredGraph<View,Val,Degree,StateIdx>::State& 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::o_state
  (int i, const typename OpenLayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return o_state(i,e.o_state);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool 
  OpenLayeredGraph<View,Val,Degree,StateIdx>::o_dec
  (int i, const typename OpenLayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return --o_state(i,e).i_deg == 0;
  }


  /*
   * Value iterator
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues::LayerValues(void) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues
  ::LayerValues(const Layer& l)
    : s1(l.support), s2(l.support+l.size) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues::init(const Layer& l) {
    s1=l.support; s2=l.support+l.size;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues
  ::operator ()(void) const {
    return s1<s2;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues::operator ++(void) {
    s1++;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  OpenLayeredGraph<View,Val,Degree,StateIdx>::LayerValues::val(void) const {
    return s1->val;
  }


  /*
   * Index advisors
   *
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::Index::Index(Space& home, Propagator& p,
                                                       Council<Index>& c,
                                                       int i0)
    : Advisor(home,p,c), i(i0) {}

  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::Index::Index(Space& home, bool share,
                                                       Index& a)
    : Advisor(home,share,a), i(a.i) {}



  /*
   * Index ranges
   *
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::IndexRange(void)
    : _fst(INT_MAX), _lst(INT_MIN) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::reset(void) {
    _fst=INT_MAX; _lst=INT_MIN;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::add(int i) {
    _fst=std::min(_fst,i); _lst=std::max(_lst,i);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::add
  (const typename OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange& ir) {
    _fst=std::min(_fst,ir._fst); _lst=std::max(_lst,ir._lst);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::empty(void) const {
    return _fst>_lst;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::lshift(int n) {
    if (empty())
      return;
    if (n > _lst) {
      reset();
    } else {
      _fst = std::max(0,_fst-n);
      _lst -= n;
    }
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::fst(void) const {
    return _fst;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  OpenLayeredGraph<View,Val,Degree,StateIdx>::IndexRange::lst(void) const {
    return _lst;
  }



  /*
   * The layered graph
   *
   */

  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>::OpenLayeredGraph(Home home,
                                                       const VarArgArray<Var>& x, 
                                                       const DFA& dfa,
                                                       Int::IntView length)
    : Propagator(home), c(home), n(0), 
      max_states(static_cast<StateIdx>(dfa.n_states())),
      n_states(0),
      n_edges(0),
      dfa(dfa),
      length(length) {
    assert(length.max() > 0);
    //home.notice(*this, AP_DISPOSE);
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  OpenLayeredGraph<View,Val,Degree,StateIdx>::audit(void) {
#ifdef GECODE_AUDIT
    // Check states and edge information to be consistent
    unsigned int n_e = 0; // Number of edges
    unsigned int n_s = 0; // Number of states
    StateIdx m_s = 0; // Maximal number of states per layer
    for (int i=n; i--; ) {
      n_s += layers[i].n_states;
      m_s = std::max(m_s,layers[i].n_states);
      for (ValSize j=layers[i].size; j--; )
        n_e += layers[i].support[j].n_edges;
    }
    n_s += layers[n].n_states;
    m_s = std::max(m_s,layers[n].n_states);
    assert(n_e == n_edges);
    assert(n_s <= n_states);
    assert(m_s <= max_states);
#endif
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline ExecStatus
    OpenLayeredGraph<View,Val,Degree,StateIdx>::extend(Space& home) {
      assert(n < length.min());
      Region r(home);
      
      // Allocate memory for all possible states in new layers
      // 1 extra state for 0 layers (only used first time)
      State* states = r.alloc<State>(1 + max_states*(length.min()-n));
      for (int i= 1 + static_cast<int>(max_states)*(length.min()-n); i--; )
        states[i].init();
      if(n==0) {
        layers[0].states = states;
        layers[0].n_states = 1;
        n_states = 1;
        // Mark initial state as being reachable
        i_state(0,static_cast<StateIdx>(0)).i_deg = 1;
        dfa_map[0]=0;
      }
      for (int i=length.min(); i > n; i--) {
        layers[i].states = states + 1 + ((i-n)-1)*max_states;
      }

      // undo outgoing edges of "final" states in previous last layer
      for (StateIdx j=0; j < layers[n].n_states; j++){
        layers[n].states[j].o_deg = 0;
      }
      
      // Allocate temporary memory for edges
      Edge* edges = r.alloc<Edge>(dfa.max_degree());
  
      DFA::Symbols sym(dfa);
      
      int* layer_map = r.alloc<int>(max_states);
      for (int i = max_states; i--; )
        layer_map[i] = -1;
      for (int i = layers[n].n_states; i--; )
        layer_map[dfa_map[i]] = i;
      
      // Forward pass: add transitions
      for (int i=n; i<length.min(); i++) {
        GECODE_ME_CHECK(layers[i].x.inter_v(home,sym,false));
        
        layers[i].support = home.alloc<Support>(layers[i].x.size());
        ValSize j=0;
        // Enter links leaving reachable states (indegree != 0)
        for (ViewValues<View> nx(layers[i].x); nx(); ++nx) {
          Degree n_edges=0;
          if (i == n) {
            for (DFA::Transitions t(dfa,nx.val()); t(); ++t)
              if (layer_map[t.i_state()] >= 0 &&
                  i_state(i,static_cast<StateIdx>(layer_map[t.i_state()])).i_deg != 0) {
                i_state(i,static_cast<StateIdx>(layer_map[t.i_state()])).o_deg++;
                o_state(i,static_cast<StateIdx>(t.o_state())).i_deg++;
                edges[n_edges].i_state = static_cast<StateIdx>(layer_map[t.i_state()]);
                edges[n_edges].o_state = static_cast<StateIdx>(t.o_state());
                n_edges++;
              }
          } else {
            for (DFA::Transitions t(dfa,nx.val()); t(); ++t)
              if (i_state(i,static_cast<StateIdx>(t.i_state())).i_deg != 0) {
                i_state(i,static_cast<StateIdx>(t.i_state())).o_deg++;
                o_state(i,static_cast<StateIdx>(t.o_state())).i_deg++;
                edges[n_edges].i_state = static_cast<StateIdx>(t.i_state());
                edges[n_edges].o_state = static_cast<StateIdx>(t.o_state());
                n_edges++;
              }
          }
          assert(n_edges <= dfa.max_degree());
          // Found support for value
          if (n_edges > 0) {
            Support& s = layers[i].support[j];
            s.val = static_cast<Val>(nx.val());
            s.n_edges = n_edges;
            s.edges = Heap::copy(home.alloc<Edge>(n_edges),edges,n_edges);
            j++;
          }
        }
        if ((layers[i].size = j) == 0)
          return ES_FAILED;
      }
    // Mark states that are "close enough" to a dfa-final state
    mindist = length.max() + 1;
    for (int s=max_states; s--; ) {
      if (o_state(length.min()-1,static_cast<StateIdx>(s)).i_deg != 0
          && distance[s] + length.min() <= length.max()) {
          o_state(length.min()-1,static_cast<StateIdx>(s)).o_deg = 1;
          mindist = std::min(mindist,distance[s]);
        }
        else
          o_state(length.min()-1,static_cast<StateIdx>(s)).o_deg = 0;
      }
      
    // Backward pass: prune all transitions that do not lead to final state
    n_edges = 0;
    for (int i=length.min(); i--; ) {
      ValSize k=0;
      for (ValSize j=0; j<layers[i].size; j++) {
        Support& s = layers[i].support[j];
        for (Degree d=s.n_edges; d--; )
          if (o_state(i,s.edges[d]).o_deg == 0) {
            // Adapt states
            i_dec(i,s.edges[d]); o_dec(i,s.edges[d]);
            // Prune edge
            s.edges[d] = s.edges[--s.n_edges];
          }
        // Value has support, copy the support information
        if (s.n_edges > 0) {
          layers[i].support[k++]=s;
          n_edges += s.n_edges;
        }
      }
      if ((layers[i].size = k) == 0)
        return ES_FAILED;
      LayerValues lv(layers[i]);
      GECODE_ME_CHECK(layers[i].x.narrow_v(home,lv,false));
      if (i >= n && !layers[i].x.assigned())
        layers[i].x.subscribe(home, *new (home) Index(home,*this,c,i));
    }
    
    // Copy and compress states, setup other information
    {
      // State map for in-states
      StateIdx* i_map = r.alloc<StateIdx>(max_states);
      // State map for out-states
      StateIdx* o_map = r.alloc<StateIdx>(max_states);
      // Number of in-states
      StateIdx i_n = 0;
      
      // Initialize map for in-states
      for (StateIdx j=max_states; j--; )
        if ((layers[length.min()].states[j].o_deg != 0) ||
            (layers[length.min()].states[j].i_deg != 0)) {
          dfa_map[static_cast<int>(i_n)] = static_cast<int>(j);
          i_map[j]=i_n++;
        }
      
      layers[length.min()].n_states = i_n;
      
      // Total number of states
      long unsigned int n_s = i_n;

      // compress the new layers
      for (int i=length.min()-1; i>n; i--) {
        // In-states become out-states
        std::swap(o_map,i_map); i_n=0;
        // Initialize map for in-states
        for (StateIdx j=max_states; j--; )
          if ((layers[i].states[j].o_deg != 0) ||
              (layers[i].states[j].i_deg != 0))
            i_map[j]=i_n++;
        layers[i].n_states = i_n;
        n_s += i_n;

        // Update states in edges
        for (ValSize j=layers[i].size; j--; ) {
          Support& s = layers[i].support[j];
          //n_edges += s.n_edges;
          for (Degree d=s.n_edges; d--; ) {
            s.edges[d].i_state = i_map[s.edges[d].i_state];
            s.edges[d].o_state = o_map[s.edges[d].o_state];
          }
        }
      }
      
      // map the (already compressed) old final layer
      std::swap(o_map,i_map);
      // Initialize map for in-states
      for (StateIdx j=0; j<layers[n].n_states; j++)
          i_map[j]=j;

      // Update states in edges
      for (ValSize j=layers[n].size; j--; ) {
        Support& s = layers[n].support[j];
        //n_edges += s.n_edges;
        for (Degree d=s.n_edges; d--; ) {
          s.edges[d].i_state = i_map[s.edges[d].i_state];
          s.edges[d].o_state = o_map[s.edges[d].o_state];
        }
      }

      // Allocate and copy states
      State* a_states = home.alloc<State>(n_s);
      for (int i=length.min(); i > n; i--) {
        StateIdx k=0;
        for (StateIdx j=max_states; j--; )
          if ((layers[i].states[j].o_deg != 0) ||
              (layers[i].states[j].i_deg != 0))
            a_states[k++] = layers[i].states[j];
        assert(k == layers[i].n_states);
        layers[i].states = a_states;
        a_states += layers[i].n_states;
      }

      n_states += n_s;
    }
    
    n = length.min();
    audit();
    return ES_OK;
  }
  
  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  forceinline ExecStatus
  OpenLayeredGraph<View,Val,Degree,StateIdx>::initialize(Space& home, const VarArgArray<Var>& x) {
                                          
    // distance[i] = shortest path from state i to a final state
    StdVectorFst fst;
    for (int i = 0; i < dfa.n_states(); i++)
      fst.AddState();
    fst.SetStart(0);
    for (int i = dfa.final_fst(); i < dfa.final_lst(); i++) {
      fst.SetFinal(i,0);
    }
    for (DFA::Transitions t(dfa); t(); ++t){
      fst.AddArc(t.i_state(), StdArc(t.symbol(), t.symbol(), 1.0, t.o_state()));
    }
    vector<TropicalWeight> d (dfa.n_states());
    ShortestDistance(fst, &d, true);
    IntArgs dist(dfa.n_states());
    for (int i = 0; i < dfa.n_states(); i++)
      dist[i] = static_cast<int>(d[i].Value());
    distance = dist;
    dfa_map = home.alloc<int>(max_states);
    // shortest accepted string gives a lower bound on length
    if (length.gq(home, distance[0]) == Int::ME_INT_FAILED)
        return ES_FAILED;
    if (!length.assigned())
      length.subscribe(home, *new (home) Index(home,*this,c,-1));
    
    // Allocate memory for layers
    // Allocate enough for length.max() up front,
    // even though only length.min()+1 layers used now.
    layers = home.alloc<Layer>(length.max()+1);
    for (int i = 0; i < length.max(); i++)
      layers[i].x = x[i];

    while (n < length.min()){
      // Add new layers
      if (extend(home) == ES_FAILED)
        return ES_FAILED;
      // Increase min length if there are no dfa-final states in last layer
      if (length.gq(home, n + mindist) == Int::ME_INT_FAILED)
          return ES_FAILED;
    }
    
    // Schedule if subsumption is needed
    if (length.assigned() || c.empty())
        View::schedule(home,*this,ME_INT_VAL);
    
    audit();
    return ES_OK;
  }

  template<class View, class Val, class Degree, class StateIdx>
  ExecStatus
  OpenLayeredGraph<View,Val,Degree,StateIdx>::advise(Space& home,
                                                 Advisor& _a, const Delta& d) {
    // Check whether state information has already been created
    if (layers[0].states == NULL) {
      State* states = home.alloc<State>(n_states);
      for (unsigned int i=n_states; i--; )
        states[i].init();
      layers[n].states = states;
      states += layers[n].n_states;
      for (int i=n; i--; ) {
        layers[i].states = states;
        states += layers[i].n_states;
        for (ValSize j=layers[i].size; j--; ) {
          Support& s = layers[i].support[j];
          for (Degree d=s.n_edges; d--; ) {
            i_state(i,s.edges[d]).o_deg++;
            o_state(i,s.edges[d]).i_deg++;
          }
        }
      }
    }
    
    Index& a = static_cast<Index&>(_a);
    const int i = a.i;

    if (i == -1) {
      // advisor is for length, not for a layer
      mindist = length.max() + 1;
      bool fix = true;
      for (int s=layers[n].n_states; s--; ) {
        if (o_state(n-1,static_cast<StateIdx>(s)).i_deg != 0
            && distance[dfa_map[s]] + n <= length.max()) {
              o_state(n-1,static_cast<StateIdx>(s)).o_deg = 1;
              mindist = std::min(mindist,distance[dfa_map[s]]);
        } else
          if (o_state(n-1,static_cast<StateIdx>(s)).o_deg != 0) {
            fix = false;
          }
          o_state(n-1,static_cast<StateIdx>(s)).o_deg = 0;
      }
      if(n + mindist > length.max())
        return ES_FAILED;
      
      if (length.min() > n) {
          return (View::modevent(d) == ME_INT_VAL) ? home.ES_NOFIX_DISPOSE(c,a) : ES_NOFIX;
      }
      else {
        if (fix)
          return (View::modevent(d) == ME_INT_VAL) ? home.ES_FIX_DISPOSE(c,a) : ES_FIX;
        else
          return (View::modevent(d) == ME_INT_VAL) ? home.ES_NOFIX_DISPOSE(c,a) : ES_NOFIX;
      }
    } else {
      if (layers[i].size <= layers[i].x.size()) {
        // Propagator has already done everything
        if (View::modevent(d) == ME_INT_VAL) {
          a.dispose(home,c);
          return c.empty() ? ES_NOFIX : ES_FIX;
        } else {
          return ES_FIX;
        }
      }

      bool i_mod = false;
      bool o_mod = false;

      if (View::modevent(d) == ME_INT_VAL) {
        Val n = static_cast<Val>(layers[i].x.val());
        ValSize j=0;
        for (; layers[i].support[j].val < n; j++) {
          Support& s = layers[i].support[j];
          n_edges -= s.n_edges;
          // Supported value not any longer in view
          for (Degree d=s.n_edges; d--; ) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
          }
        }
        assert(layers[i].support[j].val == n);
        layers[i].support[0] = layers[i].support[j++];
        ValSize s=layers[i].size;
        layers[i].size = 1;
        for (; j<s; j++) {
          Support& s = layers[i].support[j];
          n_edges -= s.n_edges;
          for (Degree d=s.n_edges; d--; ) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
          }
        }
      } else if (layers[i].x.any(d)) {
        ValSize j=0;
        ValSize k=0;
        ValSize s=layers[i].size;
        for (ViewRanges<View> rx(layers[i].x); rx() && (j<s);) {
          Support& s = layers[i].support[j];
          if (s.val < static_cast<Val>(rx.min())) {
            // Supported value not any longer in view
            n_edges -= s.n_edges;
            for (Degree d=s.n_edges; d--; ) {
              // Adapt states
              o_mod |= i_dec(i,s.edges[d]);
              i_mod |= o_dec(i,s.edges[d]);
            }
            ++j;
          } else if (s.val > static_cast<Val>(rx.max())) {
            ++rx;
          } else {
            layers[i].support[k++]=s;
            ++j;
          }
        }
        assert(k > 0);
        layers[i].size = k;
        // Remove remaining values
        for (; j<s; j++) {
          Support& s=layers[i].support[j];
          n_edges -= s.n_edges;
          for (Degree d=s.n_edges; d--; ) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
          }
        }
      } else {
        Val min = static_cast<Val>(layers[i].x.min(d));
        ValSize j=0;
        // Skip values smaller than min (to keep)
        for (; layers[i].support[j].val < min; j++) {}
        Val max = static_cast<Val>(layers[i].x.max(d));
        ValSize k=j;
        ValSize s=layers[i].size;
        // Remove pruned values
        for (; (j<s) && (layers[i].support[j].val <= max); j++) {
          Support& s=layers[i].support[j];
          n_edges -= s.n_edges;
          for (Degree d=s.n_edges; d--; ) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
          }
        }
        // Keep remaining values
        while (j<s)
          layers[i].support[k++]=layers[i].support[j++];
        layers[i].size =k;
        assert(k > 0);
      }

      audit();

      bool fix = true;
      if (o_mod && (i > 0)) {
        o_ch.add(i-1); fix = false;
       }
      if (i_mod && (i+1 < n)) {
        i_ch.add(i+1); fix = false;
      }
      if (fix) {
        if (View::modevent(d) == ME_INT_VAL) {
          a.dispose(home,c);
          return c.empty() ? ES_NOFIX : ES_FIX;
        }
        return ES_FIX;
      } else {
        return (View::modevent(d) == ME_INT_VAL)
          ? home.ES_NOFIX_DISPOSE(c,a) : ES_NOFIX;
      }
    }
  }


  template<class View, class Val, class Degree, class StateIdx>
  forceinline size_t
  OpenLayeredGraph<View,Val,Degree,StateIdx>::dispose(Space& home) {
    for (Advisors<Index> a(c); a(); ++a) {
      Index ind = static_cast<Index>(a.advisor());
      assert(ind.i >= 0);
      layers[ind.i].x.cancel(home,a.advisor());
    }
    c.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Val, class Degree, class StateIdx>
  ExecStatus
  OpenLayeredGraph<View,Val,Degree,StateIdx>::propagate(Space& home, const ModEventDelta&) {
    
    // Forward pass
    for (int i=i_ch.fst(); i<=i_ch.lst(); i++) {
      bool i_mod = false;
      bool o_mod = false;
      ValSize j=0;  // index for values with prior support
      ValSize k=0;  // index for values that still have support
      ValSize s=layers[i].size;
      do {
        Support& s=layers[i].support[j];
        n_edges -= s.n_edges;
        for (Degree d=s.n_edges; d--; )
          if (i_state(i,s.edges[d]).i_deg == 0) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
            // Remove edge
            s.edges[d] = s.edges[--s.n_edges];
          }
        n_edges += s.n_edges;
        // Check whether value is still supported
        if (s.n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,s.val));
        } else {
          layers[i].support[k++]=s;
        }
      } while (++j<s);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
      if (i_mod && (i+1 < n))
        i_ch.add(i+1);
    }
    
    /// Update lower bound of length if
    /// there is no dfa-final state in layer[n]:
    // Update the min distance to any final state   
    mindist = length.max() + 1;
    bool o_mod = false;
    for (int s=layers[n].n_states; s--; ) {
      if (o_state(n-1,static_cast<StateIdx>(s)).i_deg != 0
          && distance[dfa_map[s]] + n <= length.max()) {
            o_state(n-1,static_cast<StateIdx>(s)).o_deg = 1;
            mindist = std::min(mindist,distance[dfa_map[s]]);
      } else
        if (o_state(n-1,static_cast<StateIdx>(s)).o_deg != 0)
          o_mod = true;
        o_state(n-1,static_cast<StateIdx>(s)).o_deg = 0;
    }
    if (o_mod)
      o_ch.add(n-1);
    
    bool repeat;
    do {
      repeat = false;
      // Increase min length if there are no dfa-final states in last layer
      if (length.gq(home, n + mindist) == Int::ME_INT_FAILED)
          return ES_FAILED;
    
      if (length.assigned()) {
        VarArgArray<typename ViewTraits<View>::Var> _x;
        for (int i = 0; i < length.val(); i++){
          _x << layers[i].x;
        }
        DFA d(dfa);
        GECODE_REWRITE(*this,Int::Extensional::post_lgp(home(*this),_x,d));
      }
    
      // Extend layered graph if min length has increased
      if (n < length.min()){
        repeat = true;
        if (extend(home) == ES_FAILED)
          return ES_FAILED;
        // already did full bw pass in extend()
        a_ch.add(o_ch);
        o_ch.reset();
      }
    } while (repeat);
    
    // Backward pass
    for (int i=o_ch.lst(); i>=o_ch.fst(); i--) {
      bool o_mod = false;
      ValSize j=0;
      ValSize k=0;
      ValSize s=layers[i].size;
      do {
        Support& s=layers[i].support[j];
        n_edges -= s.n_edges;
        for (Degree d=s.n_edges; d--; )
          if (o_state(i,s.edges[d]).o_deg == 0) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            (void)   o_dec(i,s.edges[d]);
            // Remove edge
            s.edges[d] = s.edges[--s.n_edges];
          }
        n_edges += s.n_edges;
        // Check whether value is still supported
        if (s.n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,s.val));
        } else {
          layers[i].support[k++]=s;
        }
      } while (++j<s);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
    }
    
    a_ch.add(i_ch); i_ch.reset();
    a_ch.add(o_ch); o_ch.reset();

    audit();

    // Check subsumption
    if (c.empty())
      return home.ES_SUBSUMED(*this);
    
    return ES_FIX;
  }


  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  ExecStatus
  OpenLayeredGraph<View,Val,Degree,StateIdx>::post(Home home, 
                                               const VarArgArray<Var>& x,
                                               const DFA& dfa,
                                               Int::IntView length) {
    if (length.max() == 0) {
      // Check whether the start state 0 is also a final state
      if ((dfa.final_fst() <= 0) && (dfa.final_lst() >= 0))
        return ES_OK;
      return ES_FAILED;
    }
    GECODE_ME_CHECK(length.lq(home,x.size()));
    GECODE_ME_CHECK(length.gr(home,0));
    
    OpenLayeredGraph<View,Val,Degree,StateIdx>* p =
      new (home) OpenLayeredGraph<View,Val,Degree,StateIdx>(home,x,dfa,length);
    return p->initialize(home,x);
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  OpenLayeredGraph<View,Val,Degree,StateIdx>
  ::OpenLayeredGraph(Space& home, bool share,
                 OpenLayeredGraph<View,Val,Degree,StateIdx>& p)
    : Propagator(home,share,p), 
      n(p.n), layers(home.alloc<Layer>(p.length.max()+1)),
      max_states(p.max_states), n_states(p.n_states), n_edges(p.n_edges),
      mindist(p.mindist) {
    c.update(home,share,p.c);
    length.update(home,share,p.length);
    dfa.update(home,share,p.dfa);
    distance.update(home, share, p.distance);
    
    // Do not allocate states, postpone to advise!
    layers[n].n_states = p.layers[n].n_states;
    layers[n].states = NULL;
    // Allocate memory for edges
    Edge* edges = home.alloc<Edge>(n_edges);
    // Copy layers
    for (int i=length.max(); i--; ) {
      layers[i].x.update(home,share,p.layers[i].x);
    }
    for (int i=n; i--; ) {
      //layers[i].x.update(home,share,p.layers[i].x);
      assert(layers[i].x.size() == p.layers[i].size);
      layers[i].size = p.layers[i].size;
      assert(layers[i].size == p.layers[i].size);
      layers[i].support = home.alloc<Support>(layers[i].size);
      for (ValSize j=layers[i].size; j--; ) {
        layers[i].support[j].val = p.layers[i].support[j].val;
        layers[i].support[j].n_edges = p.layers[i].support[j].n_edges;
        assert(layers[i].support[j].n_edges > 0);
        layers[i].support[j].edges = 
          Heap::copy(edges,p.layers[i].support[j].edges,
                     layers[i].support[j].n_edges);
        edges += layers[i].support[j].n_edges;
      }
      layers[i].n_states = p.layers[i].n_states;
      layers[i].states = NULL;
    }
    // copy final-layer-to-dfa map
    for (int j=layers[n].n_states; j--; )
      dfa_map[j] = p.dfa_map[j];
    
    audit();
  }

  template<class View, class Val, class Degree, class StateIdx>
  PropCost
  OpenLayeredGraph<View,Val,Degree,StateIdx>::cost(const Space&,
                                               const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,n);
  }

  template<class View, class Val, class Degree, class StateIdx>
  Actor*
  OpenLayeredGraph<View,Val,Degree,StateIdx>::copy(Space& home, bool share) {
    /*
      TODO: restore elimination of assigned prefixes
    
    Eliminating an assigned prefix is still a valid idea. 
    BUT it changes the relationship between the layers of the graph and the length var.
    Would need to track the size of all eliminated prefixes, 
    and modify all references to length.min(), length.max() accordingly.
    */
    
    // // Eliminate an assigned prefix
    // {
    //   int k=0;
    //   while (layers[k].size == 1) {
    //     assert(layers[k].support[0].n_edges == 1);
    //     n_states -= layers[k].n_states;
    //     k++;
    //   }
    //   if (k > 0) {
    //     /*
    //      * The state information is always available: either the propagator
    //      * has been created (hence, also the state information has been
    //      * created), or the first variable become assigned and hence
    //      * an advisor must have been run (which then has created the state
    //      * information).
    //      */
    //     // Eliminate assigned layers
    //     n -= k; layers += k;
    //     // Eliminate edges
    //     n_edges -= static_cast<unsigned int>(k);
    //     // Update advisor indices
    //     for (Advisors<Index> as(c); as(); ++as)
    //       as.advisor().i -= k;
    //     // Update all change information
    //     a_ch.lshift(k);
    //   }
    // }
    // audit();
    
    // Compress states
    if (!a_ch.empty()) {
      int f = a_ch.fst();
      int l = a_ch.lst();
      assert((f >= 0) && (l <= n));
      Region r(home);
      // State map for in-states
      StateIdx* i_map = r.alloc<StateIdx>(max_states);
      // State map for out-states
      StateIdx* o_map = r.alloc<StateIdx>(max_states);
      // Number of in-states
      StateIdx i_n = 0;
    
      int* new_dfa_map = r.alloc<int>(dfa.n_states());
      
      n_states -= layers[l].n_states;
      // Initialize map for in-states and compress
      for (StateIdx j=0; j<layers[l].n_states; j++)
        if ((layers[l].states[j].i_deg != 0) ||
            (layers[l].states[j].o_deg != 0)) {
          layers[l].states[i_n]=layers[l].states[j];
          if (l == n) new_dfa_map[static_cast<int>(i_n)] = dfa_map[static_cast<int>(j)];
          i_map[j]=i_n++;
        }
      layers[l].n_states = i_n;
      n_states += layers[l].n_states;
      assert(i_n > 0);
      
      // Update map to dfa if last layer changed
      if (l == n)
        for (StateIdx j=layers[l].n_states; j--; )
          dfa_map[j] = new_dfa_map[j];
      
      // Update in-states in edges for last layer, if any
      if (l < n)
        for (ValSize j=layers[l].size; j--; ) {
          Support& s = layers[l].support[j];
          for (Degree d=s.n_edges; d--; )
            s.edges[d].i_state = i_map[s.edges[d].i_state];
        }
    
      // Update all changed layers
      for (int i=l-1; i>=f; i--) {
        // In-states become out-states
        std::swap(o_map,i_map); i_n=0;
        // Initialize map for in-states and compress
        n_states -= layers[i].n_states;
        for (StateIdx j=0; j<layers[i].n_states; j++)
          if ((layers[i].states[j].o_deg != 0) ||
              (layers[i].states[j].i_deg != 0)) {
            layers[i].states[i_n]=layers[i].states[j];
            i_map[j]=i_n++;
          }
        layers[i].n_states = i_n;
        n_states += layers[i].n_states;
        assert(i_n > 0);
    
        // Update states in edges
        for (ValSize j=layers[i].size; j--; ) {
          Support& s = layers[i].support[j];
          for (Degree d=s.n_edges; d--; ) {
            s.edges[d].i_state = i_map[s.edges[d].i_state];
            s.edges[d].o_state = o_map[s.edges[d].o_state];
          }
        }
      }
    
      // Update out-states in edges for previous layer, if any
      if (f > 0)
        for (ValSize j=layers[f-1].size; j--; ) {
          Support& s = layers[f-1].support[j];
          for (Degree d=s.n_edges; d--; )
            s.edges[d].o_state = i_map[s.edges[d].o_state];
        }
    
      a_ch.reset();
    }
    audit();

    return new (home) OpenLayeredGraph<View,Val,Degree,StateIdx>(home,share,*this);
  }

  /// Select small types for the layered graph propagator
  template<class Var>
  forceinline ExecStatus
  post_lgp(Home home, const VarArgArray<Var>& x, const DFA& dfa, Int::IntView length) {
    Gecode::Support::IntType t_state_idx =
      Gecode::Support::u_type(static_cast<unsigned int>(dfa.n_states()));
    Gecode::Support::IntType t_degree =
      Gecode::Support::u_type(dfa.max_degree());
    Gecode::Support::IntType t_val = 
      std::max(Support::s_type(dfa.symbol_min()),
               Support::s_type(dfa.symbol_max()));
    switch (t_val) {
    case Gecode::Support::IT_CHAR:
    case Gecode::Support::IT_SHRT:
      switch (t_state_idx) {
      case Gecode::Support::IT_CHAR:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned char,unsigned char>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned short int,unsigned char>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned int,unsigned char>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_SHRT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned char,unsigned short int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned short int,unsigned short int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned int,unsigned short int>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_INT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned char,unsigned int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned short int,unsigned int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,short int,unsigned int,unsigned int>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      default: GECODE_NEVER;
      }

    case Gecode::Support::IT_INT:
      switch (t_state_idx) {
      case Gecode::Support::IT_CHAR:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned char,unsigned char>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned short int,unsigned char>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned int,unsigned char>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_SHRT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned char,unsigned short int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned short int,unsigned short int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned int,unsigned short int>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_INT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned char,unsigned int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_SHRT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned short int,unsigned int>
            ::post(home,x,dfa,length);
        case Gecode::Support::IT_INT:
          return Extensional::OpenLayeredGraph
            <typename OVarTraits<Var>::View,int,unsigned int,unsigned int>
            ::post(home,x,dfa,length);
        default: GECODE_NEVER;
        }
        break;
      default: GECODE_NEVER;
      }

    default: GECODE_NEVER;
    }
    return ES_OK;
  }

}}}

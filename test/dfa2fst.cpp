/*
Author:   Joseph D. Scott
          Uppsala University

Used in:
          "On Constraint Solving with String Variables"
          Scott, J.D., Flener, P. and Pearson, J.
          submitted to CP 2014

*/

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <fst/fstlib.h>

using namespace Gecode;
using namespace std;
using namespace fst;

int
main(int argc, char* argv[]) {
  REG lang1 = *(REG(IntArgs(3, 1,2,3))) + REG(1) + (REG(IntArgs(3, 1,2,3)))(6,6);
  REG lang2 = *(REG(IntArgs(3, 1,2,3))) + REG(2) + (REG(IntArgs(3, 1,2,3)))(5,5);
  DFA dfa1(lang1);
  DFA dfa2(lang2);
  {
    cout << "0 (start)";
    
  
    StdVectorFst fst1;
    fst1.AddState();
    fst1.SetStart(0);
    if (0 >= dfa1.final_fst() && 0 < dfa1.final_lst()){
      fst1.SetFinal(0,0);
      cout << " (final)";
    }
    cout << endl;
  
    for (int i =1; i < dfa1.n_states(); i++){
      cout << "state " << i;
      fst1.AddState();
      if (i >= dfa1.final_fst() && i < dfa1.final_lst()) {
        fst1.SetFinal(i,0);
        cout << " (final)";
      }
      cout <<endl;
    }
    for (DFA::Transitions t(dfa1); t(); ++t){
      cout << t.i_state() << " <-" << t.symbol() << "-> " << t.o_state() << endl;
      fst1.AddArc(t.i_state(), StdArc(t.symbol(), t.symbol(), 1.0, t.o_state()));
    }
    fst1.Write("binary1.fst");
  
    vector<TropicalWeight> distance (dfa1.n_states());
    ShortestDistance(fst1, &distance, true);
    for (int i = 0; i < dfa1.n_states(); i++) {
      cout << "distance[" << i << "] = " << distance[i] << endl;
    }
  }
  {
    cout << "0 (start)";
  
    StdVectorFst fst2;
    fst2.AddState();
    fst2.SetStart(0);
    if (0 >= dfa2.final_fst() && 0 < dfa2.final_lst()){
      fst2.SetFinal(0,0);
      cout << " (final)";
    }
    cout << endl;
  
    for (int i =1; i < dfa2.n_states(); i++){
      cout << "state " << i;
      fst2.AddState();
      if (i >= dfa2.final_fst() && i < dfa2.final_lst()) {
        fst2.SetFinal(i,0);
        cout << " (final)";
      }
      cout <<endl;
    }
    for (DFA::Transitions t(dfa2); t(); ++t){
      cout << t.i_state() << " <-" << t.symbol() << "-> " << t.o_state() << endl;
      fst2.AddArc(t.i_state(), StdArc(t.symbol(), t.symbol(), 1.0, t.o_state()));
    }
    fst2.Write("binary2.fst");
  
    vector<TropicalWeight> distance (dfa2.n_states());
    ShortestDistance(fst2, &distance, true);
    for (int i = 0; i < dfa2.n_states(); i++) {
      cout << "distance[" << i << "] = " << distance[i] << endl;
    }
  }
}
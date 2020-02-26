#include "ilcConstraintAPC.h"

#include <algorithm>

#define DEBUG
#define FILTER_FAMILY

bool compareWeights(FamilyRun f1, FamilyRun f2) { return f1.getWeight() < f2.getWeight(); }


ILCCTDEMON0(Relax1SFDemon, IlcRelax1SFConstraintI, varDemon)

void IlcRelax1SFConstraintI::post() {
  IloCPEngine cp = getCPEngine();
  _f.whenRange(Relax1SFDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++) {
    _x[i].whenRange(Relax1SFDemon(cp, this));
  }
}

// set domains and count total number of required jobs
void IlcRelax1SFConstraintI::initSequence() {
  for (int i = 1; i <= _n; i++) {
    s.setRequired(i, _x[i-1].getMin());
  }
  s.sequencing();
}

void IlcRelax1SFConstraintI::reduceMaxFamily(int i) {
  const int min = _x[i-1].getMin();
  int max = _x[i-1].getMax();
  const int flow = _f.getMax();
  // std::cout << "FLOW " << flow << std::endl;
  while(max > min) {
    // std::cout << "MAX " << max << std::endl;
    s.setRequired(i, max);
    s.sequencing();
    if(s.searching(flow)) {
      _x[i-1].setMax(max);
      break;        
    }
    max--;  
  }        
  s.setRequired(i, min);
}

void IlcRelax1SFConstraintI::propagate() {
  initSequence();
  IloInt flowtime = s.searching();
// #ifdef DEBUG
//   std::cout << "FLOWTIME " <<  _f;
// #endif
  _f.setMin(flowtime);
//   #ifdef DEBUG
//   std::cout << " -> " <<  _f << std::endl;
// #endif

#ifdef FILTER_FAMILY
  for (int i = 1; i <= _n; i++) {
    const int oldMax = _x[i-1].getMax(); 
    if(! _x[i-1].isFixed()) reduceMaxFamily(i);
    const int newMax = _x[i-1].getMax(); 
    if(newMax != oldMax) {
     std::cout << "N_F" <<  i << ": " << oldMax << " -> " << newMax << std::endl;
   }
  } 
#endif  
}

void IlcRelax1SFConstraintI::varDemon() { push(); }

#include "ilcConstraintAPC.h"

#include <algorithm>

#define DEBUG
// #define FILTER_FAMILY

bool compareWeights(FamilyRun f1, FamilyRun f2) { return f1.getWeight() < f2.getWeight(); }

typedef struct Flowdata {
  int makespan;
  int flowtime;
} Flowdata;

ILCCTDEMON0(Relax1SFDemon, IlcRelax1SFConstraintI, varDemon)

void IlcRelax1SFConstraintI::post() {
  IloCPEngine cp = getCPEngine();
  _f.whenRange(Relax1SFDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++) {
    _x[i].whenRange(Relax1SFDemon(cp, this));
  }
}

// set domains and count total number of required jobs
void IlcRelax1SFConstraintI::initSWMPT() {
  for (int i = 1; i <= _n; i++) {
    s.setRequired(i, _x[i-1].getMin());
  }
  s.sequencing();
}

void IlcRelax1SFConstraintI::scheduleEmptyRun(int i) {
  #ifdef DEBUG
  std::cout << "N_F" <<  i << ": " << _x[i-1];
  #endif
  const int min = _x[i-1].getMin();
  int max = _x[i-1].getMax();
  const int flow = _f.getMax();
  while(max > min) {
    s.setRequired(i, max);
    s.sequencing();
    if(s.searching(flow)) {
      _x[i-1].setMax(max);        
    }
    max--;  
  }        
  #ifdef DEBUG
  std::cout << " -> " << _x[i-1] << std::endl;
  #endif
  s.setRequired(i, min);
}

int IlcRelax1SFConstraintI::scheduleRun(int i, int setup) {
  return 0;
}

IloInt IlcRelax1SFConstraintI::sequenceSWMPT() {
  return 0;
}

void IlcRelax1SFConstraintI::propagate() {
  initSWMPT();
  IloInt flowtime = s.searching();
#ifdef DEBUG
  std::cout << "FLOWTIME " <<  _f;
#endif
  _f.setMin(flowtime);
  #ifdef DEBUG
  std::cout << " -> " <<  _f << std::endl;
#endif

#ifdef FILTER_FAMILY
  for (int i = 1; i <= _n; i++) {
    if(! _x[i-1].isFixed()) scheduleEmptyRun(i);
  }
#endif
}

void IlcRelax1SFConstraintI::varDemon() { push(); }

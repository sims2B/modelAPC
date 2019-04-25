#include "ilcConstraintAPC.h"

#include <algorithm>

#define DEBUG

bool compareDuration(VFamily f1, VFamily f2) { return f1.weight < f2.weight; }

ILCCTDEMON0(Relax1SFDemon, IlcRelax1SFConstraintI, varDemon)

void IlcRelax1SFConstraintI::post () {
  IloCPEngine cp = getCPEngine();
  _f.whenRange(Relax1SFDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++) {
    _x[i].whenRange(Relax1SFDemon(cp, this));
  }
}

// set domains and count total number of required jobs
void IlcRelax1SFConstraintI::initSWMPT() {
  s[0].next = 0;
  for(int i = 1; i <= _n; i++) {
    s[0].next += _x[i-1].getMin();
    IlcIntVar x = _x[f[i].index];
    f[i].required = x.getMin();
    f[i].optional = x.getMax() - x.getMin();
    f[i].weight = WMPT(f[i]);
  }
  //FIXME Could not use this, should use a WSPT comparator that depends on the nomber of jobs in a block ...
  // More complicated ...
  std::sort(f.begin()++, f.end(), compareDuration);

#ifdef DEBUG
  std::cout << "Families :" << std::endl; 
  for (auto x : f) {
    printf("r=%-2d d=%-2d s=%-2d o=%-2d w=%.1f\n", x.required, x.duration, x.setup, x.optional, x.weight);
  }
#endif

  
}

void IlcRelax1SFConstraintI::scheduleEmptyRun(int i) {
  s[i].start = s[i-1].end;
  s[i].duration = 0;
  s[i].end =s[i].start;
  s[i].pred = s[i-1].pred + f[i-1].required;
  s[i].next = s[i-1].next;
}

int IlcRelax1SFConstraintI::scheduleRun(int i, int setup) {
  s[i].start = s[i-1].end + setup;
  s[i].duration = f[i].required * f[i].duration;
  s[i].end = s[i].start + s[i].duration;
  s[i].pred = s[i-1].pred + f[i-1].required;
  s[i].next = s[i-1].next - f[i].required;
  const int flowtime = f[i].required * ( (f[i].required + 1)* f[i].duration +  2 * s[i].start)/2;
#ifdef DEBUG
  printf("r:%d * %d + %d - [%3d, %3d] F=%3d\n", f[i].required, f[i].duration, f[i].setup, s[i].start,  s[i].end, flowtime);
#endif      
  return flowtime;
}

IloInt IlcRelax1SFConstraintI::sequenceSWMPT() {
  initSWMPT();
  int flowtime = 0;
  int i = 1;
  if(s[0].next > 0) {
    // Schedule blocks
    flowtime += scheduleRun(i, 0); // no initial setup
    i++;
    while(i <= _n && f[i].required > 0) {
      flowtime += scheduleRun(i, f[i].setup);
      i++;
    }
  }
  // Schedule empty blocks
  while(i <= _n) {
    // TODO Remove ? Could be in conflict with the insertion rules.
    scheduleEmptyRun(i);
    i++;
  }
  return flowtime;
}


void IlcRelax1SFConstraintI::propagate () {
  // TODO Try to Check the delta domains to avoid useless propagation
  IloInt flowtime = sequenceSWMPT();
#ifdef DEBUG
  printf("PROPAGATE %d %d -> %d\n", (int) _f.getMin(), (int) _f.getMax(), (int) flowtime);
#endif
  _f.setMin(flowtime);
}

void IlcRelax1SFConstraintI::varDemon () {
  push();
}




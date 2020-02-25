#include "ilcConstraintAPC.h"

#include <algorithm>

#define DEBUG
// #define FILTER_FAMILY

bool compareWeights(VFamily f1, VFamily f2) { return f1.weight < f2.weight; }

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
  s[0].next = 0;
  for (int i = 1; i <= _n; i++) {
    s[0].next += _x[i - 1].getMin();
    IlcIntVar x = _x[f[i].index];
    f[i].required = x.getMin();
    f[i].optional = x.getMax() - x.getMin();
    f[i].weight = WMPT(f[i]);
  }
}

void IlcRelax1SFConstraintI::scheduleEmptyRun(int i) {
  s[i].start = s[i - 1].end;
  s[i].duration = 0;
  s[i].end = s[i].start;
  s[i].pred = s[i - 1].pred + f[i - 1].required;
  s[i].next = s[i - 1].next;
}

int IlcRelax1SFConstraintI::scheduleRun(int i, int setup) {
  s[i].start = s[i - 1].end + setup;
  s[i].duration = f[i].required * f[i].duration;
  s[i].end = s[i].start + s[i].duration;
  s[i].pred = s[i - 1].pred + f[i - 1].required;
  s[i].next = s[i - 1].next - f[i].required;
  const int flowtime = f[i].required *
                       ((f[i].required + 1) * f[i].duration + 2 * s[i].start) /
                       2;
#ifdef DEBUG
  printf("r:%d * %d + %d - [%3d, %3d] F=%3d\n", f[i].required, f[i].duration,
         f[i].setup, s[i].start, s[i].end, flowtime);
#endif
  return flowtime;
}

IloInt IlcRelax1SFConstraintI::sequenceSWMPT() {
  std::sort(f.begin()++, f.end(), compareWeights);

#ifdef DEBUG
  std::cout << "Families :" << std::endl;
  for (auto x : f) {
    printf("r=%-2d d=%-2d s=%-2d o=%-2d w=%.1f\n", x.required, x.duration,
           x.setup, x.optional, x.weight);
  }
#endif
  Flowdata data = {0, 0};
  int i = 1;
  while (i <= _n && f[i].required == 0) {
    i++;
  }
  if (i >= _n) return 0;
  // Schedule the first run without setup
  data.flowtime += FLOW(f[i]);
  data.makespan += f[i].required * f[i].duration;
  i++;
  while (i <= _n) {
      data.flowtime += (data.makespan + f[i].setup) * f[i].required;
      data.flowtime += FLOW(f[i]);
      data.makespan += f[i].required * f[i].duration;
      
    i++;
  }
  return data.flowtime;
}
/*
Contre exemple : il faut mieux inverser l'ordre 
pour faire disparaitre le setup
Families :
r=0  d=0  s=0  o=0  w=0.0
r=2  d=11 s=2  o=1  w=12.0
r=3  d=12 s=9  o=0  w=15.0
*/
void IlcRelax1SFConstraintI::propagate() {
  // TODO Try to Check the delta domains to avoid useless propagation
  initSWMPT();
  IloInt flowtime = sequenceSWMPT();
#ifdef DEBUG
  printf("PROPAGATE min Flowtime: [%d, %d]-> %d\n", (int)_f.getMin(),
         (int)_f.getMax(), (int)flowtime);
#endif
  _f.setMin(flowtime);

#ifdef FILTER_FAMILY
  for (int i = 1; i <= _n; i++) {
    f[i].required += f[i].optional;
    while (f[i].optional > 0) {
      f[i].weight = WMPT(f[i]);
      IloInt flowtime = sequenceSWMPT();
      if (flowtime <= _f.getMax()) {
        break;
      }
      f[i].optional--;
      f[i].required--;
    }

#ifdef DEBUG
    printf("PROPAGATE max Family %d: %d -> %d\n", f[i].index,
           (int)_x[f[i].index].getMax(), f[i].required);
#endif
    _x[f[i].index].setMax(f[i].required);
  }
#endif
}

void IlcRelax1SFConstraintI::varDemon() { push(); }

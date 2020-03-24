#include "ilcConstraintAPC.h"

#include <algorithm>

//#define DEBUG_FLOW
#define DEBUG_FAMILY
#define DEBUG_MACHINE

bool compareWeights(FamilyRun f1, FamilyRun f2) {
  return f1.getWeight() < f2.getWeight();
}

ILCCTDEMON0(Relax1SFDemon, IlcRelax1SFConstraintI, varDemon)

void IlcRelax1SFConstraintI::post() {
  IloCPEngine cp = getCPEngine();
  _f.whenRange(Relax1SFDemon(cp, this));
  _c.whenRange(Relax1SFDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++) {
    _x[i].whenRange(Relax1SFDemon(cp, this));
  }
}

// set domains and count total number of required jobs
void IlcRelax1SFConstraintI::initSequence() {
  for (int i = 1; i <= _n; i++) {
    s.setRequired(i, _x[i - 1].getMin());
  }
}

void IlcRelax1SFConstraintI::reduceCardFamily(int i) {
  IlcIntVar x = _x[i - 1];
  if (x.isFixed())
    return;
  
  const int min = x.getMin();
  int max = x.getMax();
  const int flowUB = _f.getMax();
  do {
    s.setRequired(i, max);
    if (s.searching(flowUB)) {break;}
    max--; 
  } while (max > min);
#ifdef DEBUG_FAMILY
      if (max < x.getMax()) {
        std::cout << "\nFAMILY " << i << " FLOW_UB " << flowUB << "\nCARD " << x.getMax() << " -> "
                  << max  << std::endl;
        s.printSequence();
      }
#endif
  x.setMax(max);
  s.setRequired(i, min);
}

void IlcRelax1SFConstraintI::reduceCardFamilies() {
  for (int i = 1; i <= _n; i++) {
    reduceCardFamily(i);
  }
}

void IlcRelax1SFConstraintI::initExtendedSequence() {
  for (int i = 1; i <= _n; i++) {
    se.setRequired(i, _x[i - 1].getMin());
    se.setRequired(_n + i, 0);
  }
}

bool IlcRelax1SFConstraintI::extendSequence(int size) {
  int f = 0;
  int missing = size - se.getSize();
#ifdef DEBUG_MACHINE
  std::cout << "MISSING " << missing << std::endl;
#endif
  while (missing > 0 && f < _n) {
    IlcIntVar x = _x[orderSPT[f] - 1];
    int opt = x.getMax() - x.getMin();
#ifdef DEBUG_MACHINE
    if (opt == 0)
      std::cout << "NO MORE FAMILY " << _n + orderSPT[f] << std::endl;
    else
      std::cout << "ADD FAMILY " << _n + orderSPT[f] << std::endl;
#endif
    if (opt > missing) {
      opt = missing;
    }
    se.setRequired(_n + orderSPT[f], opt);
    missing -= opt;
    f++;
  }
  return missing == 0;
}

void IlcRelax1SFConstraintI::reduceCardMachine() {
  int f = _n - 1;
  const int flowUB = _f.getMax();

  while (!se.searching(flowUB)) {
    while (f >= 0) {
      const int i = _n + orderSPT[f];
      if (se.getRequired(i) > 0) {
        se.decrementRequired(i);
        break;
      }
      f--;
    }
    if (f >= 0) {
#ifdef DEBUG_MACHINE
      se.printSequence();
      printf("\n");
#endif
    } else
      break;
  }
  _c.setMax(se.getSize());
}

void IlcRelax1SFConstraintI::increaseFlowtime(SequenceSMPT &s) {
  const int flowLB = s.searching();
#ifdef DEBUG_FLOW
  if (_f.getMin() < flowLB) {
    std::cout << "FLOWTIME " << _f.getMin() << "->" << flowLB << std::endl;
  }
#endif
  _f.setMin(flowLB);
}

#define REQ_LB 1
#define F_CARD 2
#define EXT_LB 4
#define M_CARD 8


void IlcRelax1SFConstraintI::propagate() {
  
  if (propagationMask & (REQ_LB | F_CARD) ) {
    initSequence();
    increaseFlowtime(s);
    if (propagationMask & F_CARD) {
      for (int i = 1; i <= _n; i++) {
        reduceCardFamily(i);
      }
    }
  }
  
  if (propagationMask & EXT_LB) {
    initExtendedSequence();
    if (extendSequence(_c.getMin())) {
      increaseFlowtime(se);
    } else {
      fail();
    }
  }

  if (propagationMask & M_CARD && !_c.isFixed()) {
      initExtendedSequence();
      if (extendSequence(_c.getMax())) {
        reduceCardMachine();
      } else {
         _c.setMax(se.getSize());
      }
  }
  
}

void IlcRelax1SFConstraintI::varDemon() { push(); }

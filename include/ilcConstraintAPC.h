#ifndef ILCCONSTRAINTAPC_H
#define ILCCONSTRAINTAPC_H

#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"

#include "sequenceSMPT.h"
#ifndef __INTELLISENSE__ // code that generates an error squiggle #endif
// FIX : missing include generic.h 
#include <ilcp/cp.h>
#include <ilcp/cpext.h>
#endif
#include <vector>

// #define WMPT(f) ((double)f.duration + ((double)f.setup) / ((double)f.required))
// #define FLOW(f) ((f.required * (f.required + 1) / 2)*f.duration)


// This is simply a constraint that is pushed and that calls the execute()
// function of the above custom inferencer
class IlcRelax1SFConstraintI : public IlcConstraintI {
 protected:
  IlcInt _n;
  IlcIntVarArray _x;
  IlcIntVar _f;
  //IlcIntArray _d;
  //IlcIntArray _s;
  SequenceSMPT s;
  int propagationMask;
  
 public:
  IlcRelax1SFConstraintI(IloCPEngine cp, IlcIntVarArray families,
                         IlcIntVar flowtime, IlcIntArray durations,
                         IlcIntArray setups, int propagationMask)
      : IlcConstraintI(cp),
        _n(families.getSize()),
        _x(families),
        _f(flowtime),
        s(toVector(durations), toVector(setups)),
        propagationMask(propagationMask)
  //      _d(durations),
  //      _s(setups) 
  {
    //std::cout << "MASK " << propagationMask << std::endl;
  }

  ~IlcRelax1SFConstraintI() {
  }

  static std::vector<int> toVector(IlcIntArray t) {
    const int n = t.getSize();
    std::vector<int> v; 
    v.reserve(n);
    for (int i = 0; i < n; i++) {
      v.push_back(t[i]);
    }
    return v;
  }

  virtual void post();
  virtual void propagate();
  void varDemon();

 private:
  void initSequence();
  void reduceMaxFamily(int i);
  
};


#endif

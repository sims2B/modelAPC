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
  SequenceSMPT se;
  int propagationMask;

  std::vector<int> orderSPT;

 public:
  IlcRelax1SFConstraintI(IloCPEngine cp, IlcIntVarArray families,
                         IlcIntVar flowtime, IlcIntArray durations,
                         IlcIntArray setups, int propagationMask)
      : IlcConstraintI(cp),
        _n(families.getSize()),
        _x(families),
        _f(flowtime),
        s(toVector(durations), toVector(setups), false),
        se(toVector(durations), toVector(setups), true),
        propagationMask(propagationMask)
  //      _d(durations),
  //      _s(setups) 
  {
    for (int i = 1; i <= _n; i++) {
      orderSPT.push_back(i);
    }
    sort( orderSPT.begin(),orderSPT.end(), [&](int i,int j){return durations[i-1]<durations[j-1];} );
    // for (int i = 0; i < _n; i++) {
    //   std::cout << orderSPT[i] << " " << durations[orderSPT[i]-1] << " " << durations[i] << std::endl;
    // }
    // std::cout << std::endl << "MASK " << propagationMask << std::endl;
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
  void initExtendedSequence();
  bool extendSequence(int size);
  void reduceCardFamily(int i);
  
  void reduceCardMachine();
  
};


#endif

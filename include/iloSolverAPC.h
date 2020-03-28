#ifndef ILOSOLVERAPC_H
#define ILOSOLVERAPC_H

#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"

//#define IL_STD
#ifndef __INTELLISENSE__ // code that generates an error squiggle
// FIX : missing include generic.h 
#include <ilcplex/ilocplex.h>
#include <ilcp/cp.h>
#include <ilopl/iloopl.h>
#endif

#include <algorithm>

#include "solverAPC.h"


typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

class IloSolverAPC : public AbstractSolverAPC
{
protected:
  std::vector<Solution> solutionPool;

  Timer &timer;

  void setUp();
  
  void configure(IloEnv &env, IloCplex &cplex, ConfigAPC &config);

  void configure(IloEnv &env, IloCP &cp, ConfigAPC &config);

  void setStatus(IloAlgorithm &iloAlgo);
  
  // void setStatus(bool hasSolution, bool hasReachedTimeLimit);
  
  void tearDown(IloCplex &cplex);

  void tearDown(IloCP &cp);

  void tearDown();
  
  IloBool iloSolve(IloAlgorithm &iloAlgo);

  virtual void doSolve(IloEnv &env) = 0;

public:
  IloSolverAPC(Problem &problem, ConfigAPC &config, std::vector<Solution> &solutionPool, Timer &timer) : AbstractSolverAPC(problem, config), solutionPool(solutionPool), timer(timer){
    if(config.isFlowtimePriority()) {
      sort(this->solutionPool.begin(), this->solutionPool.end(), compareLexFQ);
    } else {
      sort(this->solutionPool.begin(), this->solutionPool.end(), compareLexQF);
    } 
  };

  void solve();
};

#endif
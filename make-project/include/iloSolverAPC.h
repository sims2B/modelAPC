#ifndef ILOSOLVERAPC_H
#define ILOSOLVERAPC_H

#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"

#include <ilcplex/ilocplex.h>
#include <ilcp/cp.h>
#include <ilopl/iloopl.h>

#include "solverAPC.h"

class IloSolverAPC : public AbstractSolverAPC
{
protected:
  std::vector<Solution> solutionPool;

  // Timer timer;

  void configure(IloEnv &env, IloCplex &cplex, ConfigAPC &config);

  void configure(IloEnv &env, IloCP &cp, ConfigAPC &config);

  virtual void doSolve(IloEnv &env, ConfigAPC &config) = 0;
  
  void setStatus(IloAlgorithm &iloAlgo);
  
  void setStatus(bool hasSolution, bool hasReachedTimeLimit);
  
  void tearDown(IloCplex &cplex);

  void tearDown(IloCP &cp);

public:
  IloSolverAPC(Problem &problem, ConfigAPC &config, std::vector<Solution> solutionPool) : AbstractSolverAPC(problem, config), solutionPool(solutionPool){
  };

  void solve();
};

#endif
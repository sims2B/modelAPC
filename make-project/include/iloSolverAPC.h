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

  virtual bool doSolve(IloEnv &env, ConfigAPC &config) = 0;
  
  void setStatus(bool hasSolution, bool hasReachedTimeLimit);
  
  void tearDown(IloCplex &cplex);

  void tearDown(IloCP &cp);

public:
  IloSolverAPC(Problem &problem, std::vector<Solution> solutionPool) : AbstractSolverAPC(problem), solutionPool(solutionPool){
  };

  void solve(ConfigAPC &config);
};

#endif
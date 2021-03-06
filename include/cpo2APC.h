#ifndef CPO2APC_H
#define CPO2APC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"

#include "iloSolverAPC.h"
#include "ilcConstraintAPC.h"

// TODO Avoid multiple typedef definitions in cpo[1-2]APC.h

class MyCustomDataSource : public IloOplDataSourceBaseI
{
  protected:
  Problem problem;
  ConfigAPC &config;

public:
  MyCustomDataSource(IloEnv &env, const Problem & problem, ConfigAPC &config) : IloOplDataSourceBaseI(env), problem(problem), config(config) {};
  void read() const;
};

class CpoSolver2APC : public IloSolverAPC
{

public:
  CpoSolver2APC(Problem &problem, ConfigAPC &config, std::vector<Solution> &solutionPool, Timer &timer) : IloSolverAPC(problem, config, solutionPool, timer)
  {
  }

  void doSolve(IloEnv &env);

private:
int checkObjValue(const IloCP& , IloOplModel&);
void createObj(IloEnv&, IloOplModel &, IloCP &);
  void modelToSol(const IloEnv &, const IloCP &, const IloOplElement &);

  void solToModel(const Solution& , IloEnv&, IloOplModel &, IloCP &);
};

#endif
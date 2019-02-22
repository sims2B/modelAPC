#ifndef CPO2APC_H
#define CPO2APC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include <ilcp/cp.h>
#include <ilopl/iloopl.h>

#include "solverAPC.h"
#include "paramModelAPC.h" //TODO Remove

// TODO Avoid multiple typedef definitions in cpo[1-2]APC.h
typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

class MyCustomDataSource : public IloOplDataSourceBaseI
{
  protected:
  Problem problem;

public:
  MyCustomDataSource(IloEnv &env, const Problem & problem) : IloOplDataSourceBaseI(env), problem(problem){};
  void read() const;
};

class CpoSolver2APC : public SeededSolverAPC
{

public:
  CpoSolver2APC(Problem problem, std::vector<Solution> solutionPool) : SeededSolverAPC(problem, solutionPool)
  {
  }

  void solve(ConfigAPC &config);

private:
  int modelToSol(const IloEnv &, const IloCP &, const IloOplElement &);

  int solToModel(Solution& , IloEnv&, IloOplModel &, IloCP &);
};

#endif
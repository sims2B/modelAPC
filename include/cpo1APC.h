#ifndef CPO1APC_H
#define CPO1APC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include "iloSolverAPC.h"
#include "paramModelAPC.h"  //TODO Remove


class CpoSolver1APC : public IloSolverAPC {
 public:
  CpoSolver1APC(Problem &problem, ConfigAPC &config,
                std::vector<Solution> &solutionPool, Timer &timer)
      : IloSolverAPC(problem, config, solutionPool, timer) {}

  void doSolve(IloEnv &env);

 private:
 int checkObjValue(const IloCP &, const  IloIntervalVarArray& ,const IloIntervalVarMatrix& );
  void modelToSol(const IloCP &, const IloIntervalVarMatrix &,
                  const IloIntervalVarMatrix &);

  void solToModel(const Solution &solution, IloIntervalVarArray &masterTask,
                  IloIntervalVarMatrix &altTasks,
                  IloIntervalVarMatrix &disqualif, IloIntervalVar &Cmax,
                  IloSolution &sol);

  void createModel(IloEnv &, IloModel &, IloIntervalVarArray &,
                   IloIntervalVarMatrix &, IloIntervalVarMatrix &,
                   IloIntervalSequenceVarArray &);

  void createVariables(IloEnv &, IloIntervalVarArray &, IloIntervalVarMatrix &,
                       IloIntervalVarMatrix &, IloIntervalSequenceVarArray &);

  void createObjective(IloEnv &, IloModel &, IloIntervalVarArray &,
                       IloIntervalVarMatrix &);

  void createConstraints(IloEnv &, IloModel &, IloIntervalVarArray &,
                         IloIntervalVarMatrix &, IloIntervalVarMatrix &,
                         IloIntervalSequenceVarArray &);
};

#endif
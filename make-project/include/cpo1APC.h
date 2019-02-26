#ifndef CPO1APC_H
#define CPO1APC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include "iloSolverAPC.h"
#include "paramModelAPC.h" //TODO Remove

typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

class CpoSolver1APC : public IloSolverAPC
{

  public:
    CpoSolver1APC(Problem &problem, ConfigAPC &config, std::vector<Solution> solutionPool) : IloSolverAPC(problem, config, solutionPool)
    {
    }

    void doSolve(IloEnv& env, ConfigAPC &config);

  private:
    void modelToSol(const IloCP &, const IloIntervalVarMatrix &,
                   const IloIntervalVarMatrix &);

    void solToModel(Solution& solution, IloIntervalVarArray &masterTask, IloIntervalVarMatrix &altTasks,
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
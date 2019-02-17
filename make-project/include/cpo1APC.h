#ifndef CPO1APC_H
#define CPO1APC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcp/cp.h>

#include "solverAPC.h"
#include "paramModelAPC.h" //TODO Remove

typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

class CpoSolver1APC : public SeededSolverAPC
{

  public:
    CpoSolver1APC(Problem problem, std::vector<Solution> solutionPool) : SeededSolverAPC(problem, solutionPool)
    {
    }

    void solve(ConfigAPC &config);

  private:
    int modelToSol(const IloCP &, const IloIntervalVarMatrix &,
                   const IloIntervalVarMatrix &);

    int solToModel(IloIntervalVarArray &masterTask, IloIntervalVarMatrix &altTasks,
                   IloIntervalVarMatrix &disqualif, IloIntervalVar &Cmax,
                   IloSolution &sol);

    int createModel(IloEnv &, IloModel &, IloIntervalVarArray &,
                    IloIntervalVarMatrix &, IloIntervalVarMatrix &,
                    IloIntervalSequenceVarArray &);

    int createVariables(IloEnv &, IloIntervalVarArray &, IloIntervalVarMatrix &,
                        IloIntervalVarMatrix &, IloIntervalSequenceVarArray &);

    int createObjective(IloEnv &, IloModel &, IloIntervalVarArray &,
                        IloIntervalVarMatrix &);

    int createConstraints(IloEnv &, IloModel &, IloIntervalVarArray &,
                          IloIntervalVarMatrix &, IloIntervalVarMatrix &,
                          IloIntervalSequenceVarArray &);
};

#endif
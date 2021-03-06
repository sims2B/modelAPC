#ifndef CPLEXAPC_H
#define CPLEXAPC_H

#pragma GCC diagnostic ignored "-Wignored-attributes"

#ifndef __INTELLISENSE__ // code that generates an error squiggle
// FIX : missing include generic.h 
#include <ilcplex/ilocplexi.h>
#endif

#include <iostream>
#include "iloSolverAPC.h"

#include "paramModelAPC.h"  //TODO Remove

typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3DMatrix;

class CplexSolverAPC : public IloSolverAPC {
 public:
  CplexSolverAPC(Problem& problem, ConfigAPC& config,
                 std::vector<Solution>& solutionPool, Timer& timer)
      : IloSolverAPC(problem, config, solutionPool, timer){};

 protected:
  void doSolve(IloEnv& env);
  int checkObjValue(const IloCplex&, const IloNumVarArray&,const IloNumVarMatrix&);
  // fill an instance of solution with the solution found by the model
  void modelToSol(const IloCplex&, const IloNumVar3DMatrix&,
                  const IloNumVar3DMatrix&, const IloNumVarMatrix&);

  void solToModel(Solution& solution, IloNumVar3DMatrix& x,
                  IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C,
                  IloNumVarArray& startVar, IloNumArray& startVal);

  void createModel(int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
                   IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
  // calcul d'une meilleure UB sur l'horizon?????
  void createVars(int, IloEnv&, IloNumVar3DMatrix&, IloNumVar3DMatrix&,
                  IloNumVarMatrix&);
  void createObj(IloEnv& env, IloModel& model, IloNumVarArray& C,
                 IloNumVarMatrix& Y);
  void createConstraints(int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
                         IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
};

#endif

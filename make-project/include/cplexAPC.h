#ifndef CPLEXAPC_H
#define CPLEXAPC_H

// #pragma GCC diagnostic ignored "-Wdeprecated-declarations" // TODO ?
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include <iostream>


#include "solverAPC.h"
#include "paramModelAPC.h" //TODO Remove


typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3DMatrix;

class CplexSolverAPC : public SeededSolverAPC {

  public:
  CplexSolverAPC(Problem problem, std::vector<Solution> solutionPool) : SeededSolverAPC(problem, solutionPool) {
  }
  
  void solve(ConfigAPC& config);
  
  private:
  void tearDown(IloCplex& cplex);

//TODO Remove
void setParam(IloEnv& env, IloCplex& cplex);

// fill an instance of solution with the solution found by the model
void modelToSol(const IloCplex&, const IloNumVar3DMatrix&,
               const IloNumVar3DMatrix&, const IloNumVarMatrix&);

// TODO Add the solution as a parameter
void solToModel(IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, 
        IloNumVarMatrix& Y, IloNumVarArray& C, IloNumVarArray& startVar, 
        IloNumArray& startVal);


void createModel(int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
                IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
//calcul d'une meilleure UB sur l'horizon?????
void createVars(int,IloEnv&, IloNumVar3DMatrix&,
               IloNumVar3DMatrix&, IloNumVarMatrix&);
void createConstraints(int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
               IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
};

#endif

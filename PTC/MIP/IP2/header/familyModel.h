#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#ifndef FAMILYMODEL_H
#define FAMILYMODEL_H

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include "Solution.h"

#ifndef TIME_LIMIT
#define time_limit 1000
#endif

//coefficient of the objective function
#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3DMatrix;

//////////////////////////////////////////
/////////////// SOLUTION /////////////////
//////////////////////////////////////////
int solve(const Problem&, Solution&);
//set some Cplex parameters for function solve
int setParam(IloEnv&,IloCplex&);


//////////////////////////////////////////
/////////// SOLUTION TREATMENT ///////////
//////////////////////////////////////////
// fill an instance of solution with the solution found by the model
int modelToSol(const Problem&, Solution&, IloCplex&, IloNumVar3DMatrix&,
	       IloNumVar3DMatrix&);
//display the solution found by cplex (value of the variable)
int displayCplexSolution(const Problem&, IloEnv&, IloCplex&, const IloNumVar3DMatrix&,
			 const IloNumVar3DMatrix&, const IloNumVarArray&);
//display results : time, status, objective, gap
int displayCplexResults(const IloCplex&, const IloNum&);
int displayCVS(const Problem& P, const Solution& s,const IloCplex& cplex,const IloNum& start);

//////////////////////////////////////////
////////////  MODEL CREATION /////////////
//////////////////////////////////////////
int createModel(const Problem&, int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
		IloNumVar3DMatrix&, IloNumVarArray&);
//calcul d'une meilleure UB sur l'horizon?????
int createVars(const Problem&, int,IloEnv&, IloNumVar3DMatrix&,
	       IloNumVar3DMatrix&);
int createConstraints(const Problem&, int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
	       IloNumVar3DMatrix&, IloNumVarArray&);
#endif

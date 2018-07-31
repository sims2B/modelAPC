#pragma warning(disable: 4996)
#ifndef FAMILYMODEL_H
#define FAMILYMODEL_H

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include <iostream>

#include "Solution.h"
#include "QualifCentric.h"
#ifndef TIME_LIMIT
#define time_limit 600
#endif

//coefficient of the objective function
#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1//P.N * P.computeHorizon()
#endif

#define withMIPStart 1
typedef int(*ptrHeur)(Problem, Solution&);
const ptrHeur heuristique = QCH;

typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3DMatrix;

//////////////////////////////////////////
/////////////// SOLUTION /////////////////
//////////////////////////////////////////
int solve(const Problem&, Solution&);
int useMIPStart(const Problem &P, Solution& s, IloEnv& env, IloCplex& cplex,
	IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C);
//set some Cplex parameters for function solve
int setParam(IloEnv&,IloCplex&);


//////////////////////////////////////////
/////////// SOLUTION TREATMENT ///////////
//////////////////////////////////////////
// fill an instance of solution with the solution found by the model
int modelToSol(const Problem&, Solution&, const IloCplex&, const IloNumVar3DMatrix&,
	       const IloNumVar3DMatrix&, const IloNumVarMatrix&);

int solToModel(const Problem& P, const Solution& s, IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C, IloNumVarArray& startVar, IloNumArray& startVal);

int displayCVS(const Problem& , const Solution&, const IloCplex&, const IloNum&);
//display the solution found by cplex (value of the variable)
int displayCplexSolution(const Problem&, IloEnv&, IloCplex&, const IloNumVar3DMatrix&,
			 const IloNumVar3DMatrix&, const IloNumVarArray&,
			 const IloNumVarMatrix&);
//display results : time, status, objective, gap
int displayCplexResults(const IloCplex&, const IloNum&);


//////////////////////////////////////////
////////////  MODEL CREATION /////////////
//////////////////////////////////////////
int createModel(const Problem&, int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
		IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
//calcul d'une meilleure UB sur l'horizon?????
int createVars(const Problem&, int,IloEnv&, IloNumVar3DMatrix&,
	       IloNumVar3DMatrix&, IloNumVarMatrix&);
int createConstraints(const Problem&, int, IloEnv&, IloModel&, IloNumVar3DMatrix&,
	       IloNumVar3DMatrix&, IloNumVarArray&, IloNumVarMatrix&);
#endif

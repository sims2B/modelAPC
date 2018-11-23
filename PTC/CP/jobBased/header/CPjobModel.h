#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "Solution.h"
#include "SchedulingCentric.h"
#include "QualifCentric.h"
#include "paramModelAPC.h"
#include <ilcp/cp.h>

#ifndef CPJOBMODEL_H
#define CPJOBMODEL_H


typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

namespace CP{
  int solve(const Problem&, Solution&);
}

//REMPLISSAGE STRUCTURE SOLUTION A PARTIR DU MODELE
int modelToSol(const Problem &, Solution&, const IloCP&, const IloIntervalVarMatrix&,
	const IloIntervalVarMatrix&); 
//A PARTIR D'UN OBJET SOLUTION, CREE UNE SOLUTION POUR LE MODELE CP (ET LUI DONNE)
int solToModel(const Problem& P, const Solution& s,
	IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
	IloIntervalVarMatrix& disqualif, IloIntervalVar& Cmax,
	IloSolution& sol);

//AFFICHAGE SOLUTION
int displayCPAIOR(const Problem&, const Solution& , const Solution&, const Solution&,const IloCP&, Clock::time_point,
	int solved);
int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, 
	const IloNum& bestObj,const IloNum& timeBestSol);//Format CSV
int printSol(const Problem& P, const IloCP& cp, const IloIntervalVarMatrix& altTasks,
	const IloIntervalVarMatrix& disqualif);//Valeur variable CP

//CREATION MODELE
int createModel(const Problem&,IloEnv&,IloModel&,IloIntervalVarArray&,
	IloIntervalVarMatrix&,IloIntervalVarMatrix&,
	IloIntervalSequenceVarArray&);
int createVariables(const Problem&, IloEnv&,IloIntervalVarArray&,IloIntervalVarMatrix&,
	IloIntervalVarMatrix&, IloIntervalSequenceVarArray&);
int createObjective(const Problem&,IloEnv&, IloModel&,IloIntervalVarArray&,
	IloIntervalVarMatrix&);
int createConstraints(const Problem&,IloEnv&,IloModel&,IloIntervalVarArray&,
	IloIntervalVarMatrix&,IloIntervalVarMatrix&,
	IloIntervalSequenceVarArray&);


    
    
  
#endif

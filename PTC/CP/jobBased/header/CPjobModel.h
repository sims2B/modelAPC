#include "Solution.h"
//#include "SchedulingCentric.h"
#include <ilcp/cp.h>

#ifndef CPJOBMODEL_H
#define CPJOBMODEL_H

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

#ifndef TIME_LIMIT
#define time_limit 180
#endif

#define withCPStart 0
typedef int(*ptrHeur)(Problem, Solution&);

int toto(Problem, Solution&);

const ptrHeur heuristique = toto;

typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

namespace CP{
  int solve(const Problem&, Solution&);
}

int modelToSol(const Problem &, Solution&, const IloCP&, const IloIntervalVarMatrix&,
	const IloIntervalVarMatrix&); 
int solToModel(const Problem& P, const Solution& s,
	IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
	IloIntervalVarMatrix& disqualif, IloIntervalVar& Cmax,
	IloSolution& sol);
int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj,const IloNum& timeBestSol);

int printSol(const Problem& P, const IloCP& cp, const IloIntervalVarMatrix& altTasks,
	const IloIntervalVarMatrix& disqualif);
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
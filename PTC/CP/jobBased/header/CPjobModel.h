#include "Solution.h"
#include <ilcp/cp.h>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;

namespace CP{
  int solve(const Problem&, Solution&);
}
  int modelToSol(const Problem &, Solution&, const IloCP&, const IloIntervalVarMatrix&,
	       const IloIntervalVarMatrix&);
int printSol();
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


    
    
  

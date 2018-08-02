#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "Solution.h"
#include <ilopl/iloopl.h>

#ifndef SCHEDAPC_H
#define SCHEDAPC_H

#ifndef TIME_LIMIT
#define time_limit 180
#endif

class MyCustomDataSource : public IloOplDataSourceBaseI {
  Problem P;
public:
  MyCustomDataSource(IloEnv& env,const Problem& _P) : IloOplDataSourceBaseI(env), P(_P) {};
  void read() const;
};

int solve(const Problem&, Solution&);

/*int modelToSol(const Problem &, Solution&, const IloCP&, const IloIntervalVarMatrix&,const IloIntervalVarMatrix&);

int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj,const IloNum& timeBestSol);
*/
    
  
#endif

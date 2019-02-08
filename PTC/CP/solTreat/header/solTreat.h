#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "Solution.h"
#include "paramModelAPC.h"
#include <ilcp/cp.h>
#include <string>
#include <chrono>
#ifndef SOLTREAT_H
#define SOLTREAT_H

using Clock = std::chrono::high_resolution_clock;

// TODO REMOVE OR IMPROVE
// Used only for solution postprocessing
#define POSITIVE_ZERO 0.00001
#define NEGATIVE_ZERO -0.00001
inline int isNul(const double& num){
return (num <= POSITIVE_ZERO && num >= NEGATIVE_ZERO);
}

std::string getStatus(const IloBool& solFound, const IloCP& cp );
int displayCPAIOR(const Problem&, const Solution& , const Solution&, 
	const Solution&,const IloCP&, Clock::time_point,const IloBool& solved);

#endif

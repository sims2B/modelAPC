#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "Solution.h"
#include "paramModelAPC.h"
#include <ilcp/cp.h>
#include <string>
#include <chrono>
#ifndef SOLTREAT_H
#define SOLTREAT_H

using Clock = std::chrono::high_resolution_clock;

std::string getStatus(const IloBool& solFound, const IloCP& cp );
int displayCPAIOR(const Problem&, const Solution& , const Solution&, 
	const Solution&,const IloCP&, Clock::time_point,const IloBool& solved);

#endif

#include "solverAPC.h"
#include <stdio.h>


void AbstractSolverAPC::setUp(ConfigAPC& config) {
  status = S_UNKNOWN;
  solutionCount = 0;
}

void AbstractSolverAPC::tearDown(ConfigAPC& config) {
   std::cout <<
   "s " << getStatus() << std::endl <<
   "d NBSOLS " << getSolutionCount() << std::endl; 
   if(hasSolution()) {
     solution.toDimacs(problem);
   }
}

#include "solverAPC.h"
#include <stdio.h>


void AbstractSolverAPC::setUp() {
  status = S_UNKNOWN;
  solutionCount = 0;
}

void AbstractSolverAPC::tearDown() {
   std::cout <<
   "s " << getStatus() << std::endl <<
   "d NBSOLS " << getSolutionCount() << std::endl; 
   if(hasSolution()) {
     solution.toDimacs();         
   }
}

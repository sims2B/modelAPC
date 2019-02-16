#include "solverAPC.h"
#include <stdio.h>


void AbstractSolverAPC::setUp(ConfigAPC& config) {
  status = S_UNKNOWN;
  solutionCount = 0;
  runtime = clock();
}

void AbstractSolverAPC::solve(ConfigAPC& config) {
  setUp(config);
  doSolve(config);
  tearDown(config);
}

void AbstractSolverAPC::tearDown(ConfigAPC& config) {
  runtime = clock() - runtime;
   std::cout <<
   "s " << getStatus() << std::endl <<
   "d NBSOLS " << getSolutionCount() << std::endl;
   printf("d RUNTIME %.3f\n", getRuntime());
}

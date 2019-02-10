#ifndef HEURISTICS_H
#define HEURISTICS_H
#include "solverAPC.h"

class ListHeuristic : public AbstractSolverAPC {

 public :
 ListHeuristic(Problem problem) : AbstractSolverAPC(problem) {
  }

  std::string solve(ConfigAPC config);
};

#endif

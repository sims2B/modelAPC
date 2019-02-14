#ifndef HEURISTICS_H
#define HEURISTICS_H
#include "solverAPC.h"

class ListHeuristic : public AbstractSolverAPC {

 public :
 ListHeuristic(Problem problem) : AbstractSolverAPC(problem) {
  }

  std::string solve(ConfigAPC& config);

  private:
  int chooseFamily(int m, std::vector<int> toSchedule);

  void treat(const int m, const int f, std::vector<int> &endLast,std::vector<int> &toSchedule, std::vector<int> &nextOfFam);

};

class SchedCentricHeuristic : public AbstractSolverAPC {

 public :
 SchedCentricHeuristic(Problem problem) : AbstractSolverAPC(problem) {
  }

  std::string solve(ConfigAPC& config);
};

class Heuristics : public AbstractSolverAPC {

 public :
 Heuristics(Problem problem) : AbstractSolverAPC(problem) {
 }

  std::string solve(ConfigAPC& config);
};


AbstractSolverAPC* makeHeuristic(Problem problem, std::string name);

#endif

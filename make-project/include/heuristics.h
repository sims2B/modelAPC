#ifndef HEURISTICS_H
#define HEURISTICS_H
#include "solverAPC.h"

class ListHeuristic : public AbstractSolverAPC {

 public :
 ListHeuristic(Problem problem) : AbstractSolverAPC(problem) {
  }
  protected:
  void doSolve(ConfigAPC& config);

  private:
  int chooseFamily(int m, std::vector<int> toSchedule);

  void treat(const int m, const int f, std::vector<int> &endLast,std::vector<int> &toSchedule, std::vector<int> &nextOfFam);

};

class SchedCentricHeuristic : public AbstractSolverAPC {

 public :
 SchedCentricHeuristic(Problem problem) : AbstractSolverAPC(problem) {
 }
protected:
void doSolve(ConfigAPC& config);

private:
void treat(const int, const int, std::vector<int>&, std::vector<int>&, std::vector<int>&);
int remainingThresh(const int&, const int&, const int&);  
int chooseFamily(const int& m, const int& t, const int& current, std::vector<int> toSchedule);
//return the family index with the minimum remaining threshold on m at time t
int famWithMinThresh(const int &m, const int& t, std::vector<int> toSchedule);
};


AbstractSolverAPC* makeHeuristic(Problem& problem, std::string name);

#endif

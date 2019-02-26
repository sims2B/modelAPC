#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "solverAPC.h"

class HeuristicAPC : public AbstractSolverAPC
{

public:
  HeuristicAPC(Problem &problem, ConfigAPC &config) : AbstractSolverAPC(problem, config)
  {
  }

  void solve()
  {
    setUp();
    doSolve();
    tearDown(); 
  };

protected:
  virtual void doSolve() = 0;
};

class ListHeuristic : public HeuristicAPC
{

public:
  ListHeuristic(Problem &problem, ConfigAPC &config) : HeuristicAPC(problem, config)
  {
  }

protected:
  void doSolve();

private:
  int chooseFamily(int m, std::vector<int> toSchedule);

  void treat(const int m, const int f, std::vector<int> &endLast, std::vector<int> &toSchedule, std::vector<int> &nextOfFam);
};

class SchedCentricHeuristic : public HeuristicAPC
{

public:
  SchedCentricHeuristic(Problem &problem, ConfigAPC &config) : HeuristicAPC(problem, config)
  {
  }

protected:
  void doSolve();

private:
  void treat(const int, const int, std::vector<int> &, std::vector<int> &, std::vector<int> &);
  int remainingThresh(const int &, const int &, const int &);
  int chooseFamily(const int &m, const int &t, const int &current, std::vector<int> toSchedule);
  //return the family index with the minimum remaining threshold on m at time t
  int famWithMinThresh(const int &m, const int &t, std::vector<int> toSchedule);
};

class QualifCentricHeuristic : public HeuristicAPC
{

public:
  QualifCentricHeuristic(Problem &problem, ConfigAPC &config) : HeuristicAPC(problem, config)
  {
  }

protected:
  void doSolve();

private:
  int treat(const int, const int, std::vector<int> &, std::vector<int> &, std::vector<int> &);
  int remainingThresh(const int &, const int &, const int &);

  /////////////// PHASE 1 //////////////////
  int schedule(std::vector<int> &endLast);
  //return the family with the minimum remaining threshold
  int chooseFamily(const int &m, const int &t, std::vector<int> toSchedule);

  ///////////// PHASE 2 //////////////////
  // TODO Return void ?
  int intraChange(std::vector<int> &endLast);

  ///////////// PHASE 3 /////////////////

  // TODO Return void ?
  int interChange(std::vector<int> &endLast);
  int addCompletion(const int &i, const int &nbJobs, const int &k,
                    const int &m, std::vector<int> endLast);
  int getBeginOfLasts(const int &last);
  void findJobMachineMatch(int k, int j, int firstOfLast, const std::vector<int> &endLast, int &machineSelected, int &jobSelected);
  ///////////// COMMON //////////////////

  //cherche sur la machine k le job qui finit à l'instant t (on l'utilise avec le completion
  // time de la machine pour avoir la derniere tache)
  int getLastOn(const int &k, const int &t);
  int addDisqualif(const int &i, const int &j, const int &m, const int &k, const int &nbJobs);
  int updateTime(const int &i, const int &j, const int &firstOfLast, const int &k, const int &m, std::vector<int> &endLast);
  void computeLastOccBefore(int m, int i, std::vector<int> &lastOccBef);
  void computeFirstOccAfter(int m, int i, std::vector<int> &firstOccAfter);
};

HeuristicAPC *makeHeuristic(Problem &problem, ConfigAPC &config, std::string name);

#endif

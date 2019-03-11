#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "solverAPC.h"

class HeuristicAPC : public AbstractSolverAPC {
 public:
  HeuristicAPC(Problem &problem, ConfigAPC &config)
      : AbstractSolverAPC(problem, config) {}

  void solve() {
    setUp();
    doSolve();
    tearDown();
  };

 protected:
  virtual void doSolve() = 0;
};

class ListHeuristic : public HeuristicAPC {
 public:
  ListHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  int chooseFamily(int m);
  void updateDisqualif();
  void schedule(const int m, const int f);
};

class SchedCentricHeuristic : public HeuristicAPC {
 public:
  SchedCentricHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  void schedule(const int, const int);
  int remainingThresh(const int &, const int &, const int &);
  void updateDisqualif();
  int chooseFamily(const int &m);
  int famWithMinThresh(const int &m, const int &t);
};

class QualifCentricHeuristic : public HeuristicAPC {
 public:
  QualifCentricHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  void schedule(const int, const int);
  int remainingThresh(const int &, const int &, const int &);

  /////////////// PHASE 1 //////////////////
  int findSchedule();
  // return the family with the minimum remaining threshold
  int chooseFamily(const int &m);
  void updateDisqualif();

  ///////////// PHASE 2 //////////////////
  void intraChange();
  void updateTime(const Job &i, const Job &j, const int &k);
  int addDisqualif(const Job &i, const Job &j, const int &m);

  ///////////// PHASE 3 /////////////////

  // TODO Return void ?
  void interChange();
  void getLastGroup(Job& deb, Job& fin, int k, int& nbJobs);
  void findJobMachineMatch(int k, const Job& deb, const Job& fin,
                           int &machineSelected, int &jobSelected, int nbJobs);

  int addCompletion(const Job &i, const int &nbJobs, const int &k, const int &m);
  int addDisqualif(const Job &i, const Job &j, const int &m, const int &k, int);
  void updateTime(const Job& i, const Job &deb, int nbJobs, int k, int m);
};

HeuristicAPC *makeHeuristic(Problem &problem, ConfigAPC &config,
                            std::string name);

#endif

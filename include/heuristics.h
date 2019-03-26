#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "solverAPC.h"

class HeuristicAPC : public AbstractSolverAPC {
 protected:
 double alpha;
 double beta;
 public:
  HeuristicAPC(Problem &problem, ConfigAPC &config);

  void solve() {
    setUp();
    doSolve();
    tearDown();
  };

 protected:
  virtual void doSolve() = 0;
  void schedule(const int f, const int m);
  void updateDisqualifLocal(int m);
  void updateDisqualifGlobal();
};

class ListHeuristic : public HeuristicAPC {
 public:
  ListHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  int chooseFamily(int m) const;
  void updateDisqualif();
};

class SchedCentricHeuristic : public HeuristicAPC {
 public:
  SchedCentricHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  int remainingThresh(const int &, const int &) const;
  int chooseFamily(const int &m) const;
  int famWithMinThresh(const int &m) const;
};

class QualifCentricHeuristic : public HeuristicAPC {
 public:
  QualifCentricHeuristic(Problem &problem, ConfigAPC &config)
      : HeuristicAPC(problem, config) {}

 protected:
  void doSolve();

 private:
  int remainingThresh(const int &, const int &) const;

  /////////////// PHASE 1 //////////////////
  int findSchedule();
  // return the family with the minimum remaining threshold
  int chooseFamily(const int &m) const;

  ///////////// PHASE 2 //////////////////
  void intraChange();
  void updateTime(const Job &i, const Job &j, const int &k);
  int addDisqualif(const Job &i, const Job &j, const int &m) const;

  ///////////// PHASE 3 /////////////////

  // TODO Return void ?
  void interChange();
  void getLastGroup(Job &deb, Job &fin, int k, int &nbJobs);
  void findJobMachineMatch(int k, const Job &deb, const Job &fin,
                           int &machineSelected, Job &jobSelected,
                           int nbJobs) const;
  int computeNewCmax(const Job &deb, const int &m, const int &k,
                     const int &nbJobs) const;
  int addCompletion(const Job &i, const int &nbJobs, const int &k,
                    const int &m) const;
  int addDisqualif(const Job &i, const Job &j, const int &m, const int &k,
                   int) const;
  void updateTime(const Job &i, const Job &deb, int nbJobs, int k, int m);
};

HeuristicAPC *makeHeuristic(Problem &problem, ConfigAPC &config,
                            std::string name);

#endif

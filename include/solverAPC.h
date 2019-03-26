#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "configAPC.h"
#include "problem.h"
#include "solution.h"
#include "utils.h"

class AbstractSolverAPC {
 protected:
  Problem problem;
  ConfigAPC& config;
  std::string status;
  Solution solution;
  int solutionCount;

 public:
  AbstractSolverAPC(Problem& problem, ConfigAPC& config)
    : problem(problem),
      config(config),
      status(S_UNKNOWN),
      solution(Solution(problem)),
      solutionCount(0) {}
      
  virtual void solve() = 0;

  inline ConfigAPC& getConfig() const { return config; }

  inline Problem getProblem() const { return problem; }

  inline std::string getStatus() const { return status; }

  inline int getSolutionCount() const { return solutionCount; }

  inline bool hasSolution() const { return getSolutionCount() > 0; }

  inline Solution getSolution() const { return solution; }

 protected:
  virtual void setUp();

  virtual void tearDown();

  void setSAT(int solutionCount = 1) {
    status = S_SAT;
    this->solutionCount = solutionCount;
  };
};

#endif

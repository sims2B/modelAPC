#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "configAPC.h"
#include "solution.h"
#include "utils.h"
#include <time.h>

class VirtualSolverAPC {
  
 public:
  
  virtual void solve(ConfigAPC& config) = 0;

  virtual Problem getProblem() const = 0;
  
  virtual std::string getStatus() const = 0;

  virtual double getRuntime() const = 0;

  virtual int getSolutionCount() const = 0;

  inline bool hasSolution() const {
    return getSolutionCount() > 0;
  }
  
  virtual Solution getSolution() const = 0;

};


class AbstractSolverAPC : public VirtualSolverAPC {
 protected:
  Problem problem;
  std::string status;
  Solution solution;
  int solutionCount;
  clock_t runtime;
 public:

  AbstractSolverAPC(Problem problem) : problem(problem), status(S_UNKNOWN), solution(Solution(problem)), solutionCount(0) {
  }

  virtual void solve(ConfigAPC& config);
  
  inline Problem getProblem() const {
    return problem;
  }

  inline std::string getStatus() const {
    return status;
  }

  inline double getRuntime() const {
    return ((double) runtime) / CLOCKS_PER_SEC;
  }
  
  inline int getSolutionCount() const {
    return solutionCount;
  }
  
  inline Solution getSolution() const {
    return solution;
  }

protected:
  virtual void setUp(ConfigAPC& config);
  virtual void doSolve(ConfigAPC& config) = 0;
  virtual void tearDown(ConfigAPC& config);

  void setSAT(int solutionCount = 1) {
    status = S_SAT;
    this->solutionCount = solutionCount;
  };

};


#endif

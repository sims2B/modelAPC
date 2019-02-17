#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "configAPC.h"
#include "solution.h"
#include "utils.h"
#include <ctime>

class VirtualSolverAPC {
  
 public:
  
  virtual void solve(ConfigAPC& config) = 0;

  virtual Problem getProblem() const = 0;
  
  virtual std::string getStatus() const = 0;

  //virtual double getRuntime() const = 0;

  virtual int getSolutionCount() const = 0;

  inline bool hasSolution() const {
    return getSolutionCount() > 0;
  }
  
  virtual Solution getSolution() const = 0;

};

class SolverTimer {
   private:
  std::clock_t startTime;
  std::clock_t stageTime;
  std::clock_t stopTime;
 
  public:
  void startTimer() {
    startTime = std::clock();
  };
  void stageTimer() {
    stageTime = std::clock();
  };
  void stopTimer() {
    stopTime = std::clock();
  };

  float getSetupTime() {
    return ((double) (stageTime - startTime)) / CLOCKS_PER_SEC;
  };

  float getSolveTime() {
    return ((double) (stopTime - stageTime)) / CLOCKS_PER_SEC;
  };
  
  float getTotalTime() {
    return ((double) (stopTime - startTime)) / CLOCKS_PER_SEC;
  };

  void toDimacs() {
     printf("d SETUP_TIME %.3f\nd RUNTIME %.3f\nd WCTIME %.3f\n", getSetupTime(), getSolveTime(), getTotalTime());

  };
  

};


class AbstractSolverAPC : public VirtualSolverAPC {
 protected:
  Problem problem;
  std::string status;
  Solution solution;
  int solutionCount;
  SolverTimer timer;
 public:

  AbstractSolverAPC(Problem problem) : problem(problem), status(S_UNKNOWN), solution(Solution(problem)), solutionCount(0) {
  }

  
  inline Problem getProblem() const {
    return problem;
  }

  inline std::string getStatus() const {
    return status;
  }
  
  inline int getSolutionCount() const {
    return solutionCount;
  }
  
  inline Solution getSolution() const {
    return solution;
  }

protected:
  virtual void setUp(ConfigAPC& config);
  
  virtual void tearDown(ConfigAPC& config);

  void setSAT(int solutionCount = 1) {
    status = S_SAT;
    this->solutionCount = solutionCount;
  };

  void setERROR() {
    status = S_ERROR;
  };

};

#endif

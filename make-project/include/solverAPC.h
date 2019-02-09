#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "solution.h"


class AbstractSolverAPC {
 private:
  Problem problem;
  Solution solution;
  
 public:
  virtual void setUp(Problem problem);
  virtual void solve();
  virtual std::string getStatus();
  virtual bool hasSolution();
  virtual Solution getSolution();
  virtual void tearDown();

};

#endif

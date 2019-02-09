#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "solution.h"
#include "utils.h"
#include <libconfig.h++>

enum Status {UNSAT, SAT, OPTIMUM, UNKNOWN, ERROR};

enum Heuristic {LIST, SCHED_CENTRIC};

enum SolverType {HEURISTIC, CPLEX, SCHED1, SCHED2};

enum Objective {FLOWTIME, DISQUALIFIED};

enum ObjectiveType {MONO, LEXICOGRAPHIC, WEIGHTED_SUM};

//  class ConfigAPC {
//    private:
//     libconfig::Config config;

//   public:
//     void readFromFile(std::string configPath);
//     int getTimeLimit();
//     int getWorkers();
//     bool isVerbose();
    
//     SolverType getSolverType();
//     Heuristic getHeuritic();
//     std::vector<Heuristic> getHeuritics();

//     ObjectiveType getObjectiveType();
//     Objective getPriorityObjective();
    
//     std::string getModelPath();
//     bool withRelaxation1SF();

//     std::string toDimacs();
//  };

class VirtualSolverAPC {
  
 public:
  
  virtual Status solve(libconfig::Config config);

  virtual Problem getProblem();
  
  virtual Status getStatus();

  virtual int getSolutionCount();

  inline bool hasSolution() {
    return getSolutionCount() > 0;
  }
  
  virtual Solution getSolution();

};


class AbstractSolverAPC : public VirtualSolverAPC {
 private:
  Problem problem;
  Status status;
  Solution solution;
  int solutionCount;
 public:

  AbstractSolverAPC(Problem problem) : problem(problem), status(Status::UNKNOWN), solution(Solution(problem)), solutionCount(0) {
  }
  
  inline Problem getProblem() {
    return problem;
  }

  inline Status getStatus() {
    return status;
  }
  
  inline int getSolutionCount() {
    return solutionCount;
  }
  
  inline Solution getSolution() {
    return solution;
  }

};

class AbstractIloSolverAPC : public AbstractSolverAPC {
  
public :
  AbstractIloSolverAPC(Problem problem) : AbstractSolverAPC(problem) {
  }
     
 protected:
  virtual void setUp();
  virtual void solveIlo();
  virtual void tearDown();
};




#endif

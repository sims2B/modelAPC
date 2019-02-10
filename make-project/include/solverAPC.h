#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "solution.h"
#include "utils.h"
#include <libconfig.h++>
#include <vector>

#define S_UNSAT "UNSAT"
#define S_SAT "SAT"
#define S_OPTIMUM "OPTIMUM"
#define S_UNKNOWN "UNKNOWN"
#define S_ERROR "ERROR"


#define T_HEURISTIC "HEURISTIC"
#define T_CPLEX "CPLEX"
#define T_SCHED1 "SCHED1" 
#define T_SCHED2 "SCHED2"

#define O_FLOWTIME "FLOWTIME"
#define O_DISQUALIFIED "DISQUALIFIED"

#define M_MONO "MONO"
#define M_LEXICOGRAPHIC "LEX"
#define M_WEIGHTED_SUM "SUM"


 class ConfigAPC {
   private:
    libconfig::Config config;

  public:
  bool readFile(std::string configPath);
  inline int getTimeLimit() {
    return getIntValue("solver", "timeLimit", -1);
  }

  inline int getWorkers() {
    return getIntValue("solver", "workers", -1);
  }

  inline bool isVerbose() {
    return getIntValue("solver", "verbose", 0);
  }

  std::string getSolverType() {
    return getStringValue("solver", "type", T_HEURISTIC);
  }

  std::string getObjectiveType() {
    return getStringValue("objective", "type", M_MONO);
  }

  std::string getPriorityObjective() {
    return getStringValue("objective", "priority", O_FLOWTIME);
  }

  std::vector<std::string> getHeuristics();

  std::string getModelPath() {
    return getStringValue("cpo", "model", "schedAPC.mod");
  }

  bool withRelaxation1SF() {
    return getIntValue("cpo", "relax1SF", 0);
  }

  void toDimacs();

  
  private:
  int getIntValue(const char* name1, const char* name2, int defVal);
  std::string getStringValue(const char* name1, const char* name2, std::string defVal);
    
//     Heuristic getHeuritic();
//     std::vector<Heuristic> getHeuritics();

//     ObjectiveType getObjectiveType();
//     Objective getPriorityObjective();
    
 };

class VirtualSolverAPC {
  
 public:
  
  virtual std::string solve(ConfigAPC config) = 0;

  virtual Problem getProblem() = 0;
  
  virtual std::string getStatus() = 0;

  virtual int getSolutionCount() = 0;

  inline bool hasSolution() {
    return getSolutionCount() > 0;
  }
  
  virtual Solution getSolution() = 0;

};


class AbstractSolverAPC : public VirtualSolverAPC {
 private:
  Problem problem;
  std::string status;
  Solution solution;
  int solutionCount;
 public:

  AbstractSolverAPC(Problem problem) : problem(problem), status(S_UNKNOWN), solution(Solution(problem)), solutionCount(0) {
  }
  
  inline Problem getProblem() {
    return problem;
  }

  inline std::string getStatus() {
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

#ifndef SOLVERAPC_H
#define SOLVERAPC_H

#include "problem.h"
#include "solution.h"
#include "utils.h"
#include <time.h>
#include <libconfig.h++>
#include <vector>

#define S_UNSAT "UNSAT"
#define S_SAT "SAT"
#define S_OPTIMUM "OPTIMUM"
#define S_UNKNOWN "UNKNOWN"
#define S_ERROR "ERROR"


#define T_HEURISTIC "HEURISTICS"
#define T_CPLEX "CPLEX"
#define T_SCHED1 "SCHED1" 
#define T_SCHED2 "SCHED2"

#define H_LIST "LIST"
#define H_SCHED "SCHED"
#define H_QUALIF "QUALIF"

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
  
  std::string getConfigName();

  inline int getWorkers() {
    return getIntValue("solver", "workers", -1);
  }

  inline bool isVerbose() {
    return getIntValue("solver", "verbose", 0);
  }

  std::string getSolverType() {
    return getStringValue("solver", "type", T_HEURISTIC);
  }

  std::string getObjectiveFunction() {
    return getStringValue("objective", "function", M_MONO);
  }

  int getWeightFlowtime() {
    return getIntValue("objective", "weightFlowtime", 1);
  }

  int getWeightQualified() {
    return getIntValue("objective", "weightQualified", 1);
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
        
 };

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

  void solve(ConfigAPC& config);
  
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

};


#endif

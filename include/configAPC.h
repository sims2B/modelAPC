#ifndef CONFIGAPC_H
#define CONFIGAPC_H

#include <libconfig.h++>
#include <vector>
#include <iostream>

#define S_UNSAT "UNSAT"
#define S_SAT "SAT"
#define S_OPTIMUM "OPTIMUM"
#define S_UNKNOWN "UNKNOWN"
#define S_ERROR "ERROR"


#define T_NONE "NONE"
#define T_CPLEX "CPLEX"
#define T_CPO1 "CPO1" 
#define T_CPO2 "CPO2"

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
    return getBoolValue("solver", "verbose", true);
  }

  inline bool isSilent() {
    return !isVerbose();
  }

  inline bool useTikzExport() {
    return getBoolValue("solver", "tikz", false);
  }

  std::string getSolverType() {
    return getStringValue("solver", "type", T_NONE);
  }

  std::string getObjectiveFunction() {
    return getStringValue("objective", "function", M_MONO);
  }

  int getWeightFlowtime() {
    return getIntValue("objective", "weightFlowtime", 1);
  }

  bool isFlowtimePriority() {
    return getWeightFlowtime() > getWeightQualified(); 
  }

  bool isQualifiedPriority() {
    return getWeightFlowtime() <= getWeightQualified(); 
  }

  int getWeightQualified() {
    return getIntValue("objective", "weightQualified", 1);
  }

  std::vector<std::string> getHeuristics();

  std::string getModelPath() {
    return getStringValue("cpo", "model", "schedAPC.mod");
  }

  int withRelaxation1SF() {
    return getIntValue("cpo", "withRelax1SF", 3);
  }

  void toDimacs();

  
  private:
  bool getBoolValue(const char* name1, const char* name2, bool defVal);  
  int getIntValue(const char* name1, const char* name2, int defVal);
  std::string getStringValue(const char* name1, const char* name2, std::string defVal);
        
 };


#endif

#include <cstdlib>
#include <iostream>
#include <vector>
#include "cplexAPC.h"
#include "cpo1APC.h"
#include "cpo2APC.h"
#include "heuristics.h"
#include "utils.h"

AbstractSolverAPC *makeSolverAPC(Problem &problem, ConfigAPC &config,
                                 std::vector<Solution> &solutionPool,
                                 Timer &timer) {
  std::string type = config.getSolverType();
  if (type == T_CPLEX)
    return new CplexSolverAPC(problem, config, solutionPool, timer);
  else if (type == T_CPO1)
    return new CpoSolver1APC(problem, config, solutionPool, timer);
  else if (type == T_CPO2) {
    return new CpoSolver2APC(problem, config, solutionPool, timer);
  }
  return NULL;
}

int exitOnFailure(std::string message) {
  std::cerr << "ERROR: " << message << std::endl;
  std::cout << "s " << S_ERROR << std::endl;
  return (EXIT_FAILURE);
}


/*
 Arguments : configuration path ; instance path
*/
int main(int argc, char *argv[]) {
  // Start Timer
  Timer timer;
  // Check arguments
  if (argc != 3) {
    return exitOnFailure("invalid arguments");
  }
  // Print parameters
  std::string configPath = argv[1];
  std::string instancePath = argv[2];
  std::cout << "i " << getFilename(instancePath, false) << std::endl;
  std::cout << "c CONFIG " << getFilename(configPath, false) << std::endl;
  // Read Config From file
  ConfigAPC config;
  if (!config.readFile(configPath)) return exitOnFailure("invalid configuration file");
  std::ifstream instance(instancePath, std::ios::in);
  config.toDimacs();

  // Read problem instance from file
  if (!instance.is_open()) return exitOnFailure("invalid instance file");
  Problem problem = oldReader(instance);
  instance.close();
  problem.toDimacs();

  // Execute heuristics for warm getStart
  timer.stage();
  std::vector<Solution> solutionPool;
  std::vector<std::string> heuristics = config.getHeuristics();
  for (auto &heuristic : heuristics) {
    std::cout << std::endl << "d HEURISTIC " << heuristic << std::endl;
    HeuristicAPC *solver = makeHeuristic(problem, config, heuristic);
    solver->solve();/* 
    std::cout << solver->getSolution().toString();
    solver->getSolution().toTikz();  */
    if (solver->hasSolution()) {
      solutionPool.push_back(solver->getSolution());
    }
  }
  timer.stage("HEUR_TIME");
  AbstractSolverAPC *solver =
      makeSolverAPC(problem, config, solutionPool, timer);
  if (solver != NULL) {
    std::cout << std::endl;
    solver->solve(); 
  } else {
    return exitOnFailure("invalid solver type");
  }
  timer.stagewc();
  timer.toDimacs();
  return (EXIT_SUCCESS);
}

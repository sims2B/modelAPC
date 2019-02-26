#include "utils.h"
#include "heuristics.h"
#include "cplexAPC.h"
#include "cpo1APC.h"
#include "cpo2APC.h"
#include <iostream>
#include <cstdlib>
#include <vector>

VirtualSolverAPC *makeSolverAPC(Problem &problem, ConfigAPC &config, std::vector<Solution> &solutionPool)
{
  std::string type = config.getSolverType();
  if (type == T_CPLEX)
    return new CplexSolverAPC(problem, solutionPool);
  else if (type == T_CPO1)
    return new CpoSolver1APC(problem, solutionPool);
  else if (type == T_CPO2)
  {
     return new CpoSolver2APC(problem, solutionPool);
  }
  return NULL;
}

int exitOnFailure()
{
  std::cout << "s " << S_ERROR << std::endl;
  return (EXIT_FAILURE);
}
int main(int argc, char *argv[])
{
  if(argc != 3) {
     return exitOnFailure();
  }
  // Print parameters
  std::string configPath = argv[1];
  std::string instancePath = argv[2];
  std::cout << "i " << getFilename(instancePath, false) << std::endl;
  std::cout << "c CONFIG " << getFilename(configPath, false) << std::endl;
  Timer timer;
  timer.start();
  // Read Config From file
  ConfigAPC config;
  if (!config.readFile(configPath))
    return exitOnFailure();
  std::ifstream instance(instancePath, std::ios::in);
  if (!instance.is_open())
    return exitOnFailure();

  Problem problem = readFromFile(instance);
  instance.close();

  // Log on Config and Problem

  config.toDimacs();
  problem.toDimacs();

  // Execute heuristics for warm start
  timer.stage();
  std::vector<Solution> solutionPool;
  for (auto &heuristic : config.getHeuristics())
  {
    std::cout << std::endl
              << "d HEURISTIC " << heuristic << std::endl;
    HeuristicAPC *solver = makeHeuristic(problem, heuristic);
    solver->solve(config);
    if (solver->hasSolution())
    {
      solutionPool.push_back(solver->getSolution());
    }
  }
  timer.stage("HEUR_TIME");
  // TODO Pass the timer as an argument
  VirtualSolverAPC *solver = makeSolverAPC(problem, config, solutionPool); 
  if (solver != NULL)
  {
    std::cout << std::endl;
    solver->solve(config);
  } 
  timer.stop();
  timer.toDimacs();
  return (EXIT_SUCCESS);
}

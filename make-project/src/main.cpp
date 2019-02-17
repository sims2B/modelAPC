#include "utils.h"
#include "heuristics.h"
#include "cplexAPC.h"
#include "cpo1APC.h"
#include <iostream>
#include <cstdlib>
#include <vector>

VirtualSolverAPC *makeSolverAPC(Problem &problem, ConfigAPC &config, std::vector<Solution>& solutionPool)
{
  std::string type = config.getSolverType();
  if (type == T_CPLEX)
    return new CplexSolverAPC(problem, solutionPool);
  else if (type == T_CPO1)
    return new CpoSolver1APC(problem, solutionPool);
  else if (type == T_CPO2)
  {
  }
  return NULL;
}

int main(int, char *argv[])
{
  // Print parameters
  std::string configPath = argv[1];
  std::string instancePath = argv[2];
  std::cout << "i " << getFilename(instancePath, false) << std::endl;
  std::cout << "c CONFIG " << getFilename(configPath, false) << std::endl;
  // Read Config From file
  ConfigAPC config;
  if (!config.readFile(configPath))
  {
    std::cout << "s " << S_ERROR << std::endl;
    return (EXIT_FAILURE);
  }
  config.toDimacs();
  // Read Instance From file
  std::ifstream instance(instancePath, std::ios::in);
  if (instance.is_open())
  {
    Problem problem = readFromFile(instance);
    instance.close();
    problem.toDimacs();

    std::vector<Solution> solutionPool;
    for (auto &heuristic : config.getHeuristics())
    {
      std::cout << "d HEURISTIC " << heuristic << std::endl;
      HeuristicAPC *solver = makeHeuristic(problem, heuristic);
      solver->solveWithoutTimer(config);
      if (solver->hasSolution())
      {
        solutionPool.push_back(solver->getSolution());
      }
    }
    VirtualSolverAPC *solver = makeSolverAPC(problem, config, solutionPool);
    if (solver != NULL)
    {
      solver->solve(config);
      // if(solver->hasSolution()) {
      //   solver->solToModel.toTikz(problem);
      // }
    }
    else
    {
      std::cout << "s " << S_ERROR << std::endl;
      return (EXIT_FAILURE);
    }
  }
  else
  {
    std::cout << "s " << S_ERROR << std::endl;
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}

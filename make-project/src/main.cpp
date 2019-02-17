#include "utils.h"
#include "heuristics.h"
#include "cplexAPC.h"
#include <iostream>
#include <cstdlib>

VirtualSolverAPC* makeSolverAPC(Problem& problem, ConfigAPC& config) {
     return makeHeuristic(problem, H_SCHED);
     //return new CplexSolverAPC(problem);
}

int main(int,char* argv[]){
  // Print parameters
  std::string configPath = argv[1];
  std::string instancePath = argv[2];
  std::cout << "i " << getFilename(instancePath, false) << std::endl;
  std::cout << "c CONFIG " << getFilename(configPath, false) << std::endl;
  // Read Config From file
  ConfigAPC config;
  if( ! config.readFile(configPath) ) {
    std::cout << "s "<< S_ERROR << std::endl;
    return(EXIT_FAILURE);
  }
  config.toDimacs();
  // Read Instance From file
  std::ifstream instance(instancePath, std::ios::in);
  if(instance.is_open() ) {
    Problem problem = readFromFile(instance);
    instance.close();
    problem.toDimacs();
    VirtualSolverAPC* solver = makeSolverAPC(problem, config);
    solver->solve(config);
    if(solver->hasSolution()) {
      solver->getSolution().toDimacs(problem);
    }
  } else {
    std::cout << "s "<< S_ERROR << std::endl;
    return(EXIT_FAILURE);
  }
  return(EXIT_SUCCESS);
}

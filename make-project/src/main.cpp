#include "heuristics.h"
#include "utils.h"
#include <iostream>
#include <cstdlib>

VirtualSolverAPC* makeSolverAPC(Problem& problem, ConfigAPC& config) {
     ListHeuristic* h = new ListHeuristic(problem);
     return h;
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
  Problem problem = readFromFile(instance);
  if(instance.is_open() ) {
    Problem P = readFromFile(instance);
    instance.close();
     VirtualSolverAPC* solver = makeSolverAPC(problem, config);
     solver->solve(config);
     std::cout << "s "<< solver->getStatus() << std::endl;
    
  } else {
    std::cout << "s "<< S_ERROR << std::endl;
    return(EXIT_FAILURE);
  }
  
  std::cout << "s "<< S_UNKNOWN << std::endl;
  return(EXIT_SUCCESS);
}

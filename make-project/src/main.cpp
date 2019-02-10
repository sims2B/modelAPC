#include "heuristics.h"
#include "utils.h"
#include <iostream>
#include <cstdlib>

int main(int,char* argv[]){
  std::string configPath = argv[1];
  std::string instancePath = argv[2];
  // Read Instance From file
  std::ifstream instance(instancePath, std::ios::in);
  std::cout << "i " << getFilename(argv[1], false) << std::endl;
  std::cout << "c CONFIG " << getFilename(argv[1], false) << std::endl;
  // Read Config From file
  ConfigAPC config;
  if( ! config.readFile(configPath) ) {
    std::cout << "s "<< S_ERROR << std::endl;
    return(EXIT_FAILURE);
  }

  std::cout << config.getTimeLimit() << std::endl;
  // if (instance.is_open()){
    // Problem P = readFromFile(instance);
    // instance.close();
    //std::cout << P.toString();
    // std::cout << "i " << getFilename(argv[1], false) << std::endl;
    // Solution s(P);
    // solve(P, s);
  
    /*std::cout << s.toString(P);
      s.toTikz(P);*/
  //}
  std::cout << "s "<< S_UNKNOWN << std::endl;
    return(EXIT_SUCCESS);
}

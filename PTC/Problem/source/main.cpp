#include "Problem.h"

int main( int ,char* argv[]){
  std::ofstream output("inst_"+ (std::string)argv[1] + "_1_" + (std::string)argv[2] + "_10_"  + (std::string)argv[2]+"_Bthr.txt" , std::ios::out);
  Problem P = generateProblem(atoi(argv[1]),1,atoi(argv[2]),10,atoi(argv[2]));
  problem.writeInFile(output);
  return 0;
}

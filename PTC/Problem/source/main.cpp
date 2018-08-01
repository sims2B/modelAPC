#include "Problem.h"

int main( int ,char* argv[]){
  std::ofstream output(argv[1], std::ios::out);
  Problem P = generateProblem(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),
			      atoi(argv[5]),atoi(argv[6]));
  P.writeInFile(output);
  return 0;
}

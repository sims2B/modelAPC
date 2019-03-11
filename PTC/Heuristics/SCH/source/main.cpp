#include "SchedulingCentric.h"
#include "Problem.h"

int main(int, char* argv[]){

  std::ifstream instance(argv[1], std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();

  std::cout << problem.N << ";" << problem.M << ";" << problem.getNbFams() << ";";
  Solution s(P);
  SCHWithOutput(P, s);
  //std::cout << problem.toString() << s.toString();
  //s.toTikz(P);
  return 0;
}

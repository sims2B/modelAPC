
#include "QualifCentric.h"
#include "Solution.h"

int main(int, char* argv[]){

  std::ifstream instance(argv[1], std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();

  std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() << ";";
  Solution s(P);
  QCHWithOutput(P, s);
  //std::cout << P.toString() << s.toString(P);
  // s.toTikz(P);
  return 0;
}



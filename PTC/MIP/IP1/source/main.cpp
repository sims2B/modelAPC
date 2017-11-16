#include "jobModel.h"
#include <iostream>
#include <fstream>
using namespace IP1;

int main(int,char* argv[]){
  std::ifstream instance(argv[1],std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();
  std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() <<";" ;
  Solution s(P);  
  if (!solve(P,s))
    std::cout << ";" << s.isValid(P) << std::endl; 
  else  
    std::cout << "time_limit;0; ; ; ; ; ; ; ; ; \n";
  return 0;
}

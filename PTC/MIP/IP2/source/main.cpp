#include "familyModel.h"
#include <iostream>
#include <fstream>

int main(int,char *argv[]){

  std::ifstream instance(argv[1],std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();

  Solution s(P);  
  if (!solve(P,s)){
    std::cout << "Hurray!\n";
    std::cout << "valide? " << s.isValid(P) << std::endl; 
 
  }
  else  
    std::cout << "...\n";
  std::cout << "**************************************************" <<std::endl;

  return 0;

}

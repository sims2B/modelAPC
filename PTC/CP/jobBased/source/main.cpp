#include "CPjobModel.h"
#include <iostream>
#include <vector>
#include <fstream>
using namespace CP;
typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;


int main(int,char* argv[]){
 


  std::ifstream instance(argv[1],std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();
  

  Solution s(P);  
  if (!solve(P,s)){
    std::cout << "Hurray!\n"; 
    std::cout << "valide? " << s.isValid(P) << std::endl;   
    std :: cout << s.toString() << std::endl;
  }
  else  
    std::cout << "...\n";
  std::cout << "**************************************************" <<std::endl;
  /*
    
   
    Solution s(P);  
    if (!solve(P,s)){
    std::cout << "Hurray!\n"; 
    std::cout << "valide? " << s.isValid(P) << std::endl; 
 
    }
    else  
    std::cout << "...\n";
    std::cout << "**************************************************" <<std::endl;
  */
  return 0;
}

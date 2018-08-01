#include "utils.h"


void printVector(std::string name , std::vector<int> v){
  unsigned int i ;
  std::cout << name << " : [ " ;
  for (i = 0 ; i < v.size() - 1; i++)
    std::cout << v[i] << " , ";
  std::cout << v[i] << " ]\n";
}


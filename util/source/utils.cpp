#include "utils.h"




std::string getBasename(const std::string& str){
    //std::string str("abc:def:ghi:jkl"); -> exemple
    std::istringstream split(str); // flux d'exatraction sur un std::string
    std::vector<std::string> tokens;
    for (std::string each; std::getline(split, each, '/'); tokens.push_back(each));
    return tokens[tokens.size()-1];
}

void printVector(std::string name , std::vector<int> v){
  unsigned int i ;
  std::cout << name << " : [ " ;
  for (i = 0 ; i < v.size() - 1; i++)
    std::cout << v[i] << " , ";
  std::cout << v[i] << " ]\n";
}


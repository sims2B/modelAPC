#include "schedAPC.h"
#include "utils.h"
#include <filesystem> 
  
int main(int,char* argv[]){
 
  std::ifstream instance(argv[1], std::ios::in);
  if (instance.is_open()){
    Problem P = readFromFile(instance);
    instance.close();
    //std::cout << problem.toString();
    std::cout << "i " << getFilename(argv[1], false) << std::endl;
    Solution s(P);

    solve(P, s);
  
    /*std::cout << s.toString(P);
      s.toTikz(P);*/
  }
  else std::cout << "Unable to open file";
  return 0;
}


#include "CPjobModel.h"
#include "utils.h"
using namespace CP;


int main(int,char* argv[]){
 
  std::ifstream instance(argv[1], std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();
  //std::cout << P.toString();
  std::cout << "i " << getBasename(argv[1]) << std::endl;
  Solution s(P);

  solve(P, s);
  
  /*std::cout << s.toString(P);
    s.toTikz(P);*/
	
  return 0;
}

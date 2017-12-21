#include "SchedulingCentric.h"
#include "Problem.h"
#include <sys/time.h>

int main(int,char* argv[]){
  struct timeval tim;

  std::ifstream instance(argv[1],std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();

  std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() <<";" ;
  Solution s(P);
  
 gettimeofday(&tim,NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  bool solved = SCH(P,s);
  
  gettimeofday(&tim,NULL);
  double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
  
  std::cout << t2-t1 << ";" ;
  displayCVS(P,s,solved);
  std::cout << ";" << s.isValid(P) << std::endl;
  //std::cout << P.toString() << s.toString();
  return 0;
}

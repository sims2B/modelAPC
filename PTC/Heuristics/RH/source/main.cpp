#include "Recursive.h"
#include "Problem.h"
#include "ListHeuristic.h"
//#include "SchedulingCentric.h"
//#include "QualifCentric.h"
#include <sys/time.h>

int main(int,char* argv[]){
  struct timeval tim;

  std::ifstream instance(argv[1],std::ios::in);
  Problem P = readFromFile(instance);
  instance.close();

  std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() <<";" ;
  Solution s(P);
  
  ptrHeur heuristic ;
  heuristic = LH;
  
  gettimeofday(&tim,NULL);
  double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  bool solved = RH(P,s,heuristic);
  
  gettimeofday(&tim,NULL);
  double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
  
  std::cout << t2-t1 << ";" ;
  displayCVS(P,s,solved);
  std::cout << ";" << s.isValid(P) << std::endl;
  //std::cout << P.toString() << s.toString();
  return 0;
}

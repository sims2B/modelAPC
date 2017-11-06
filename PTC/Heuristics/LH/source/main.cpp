#include "ListHeuristic.h"
#include "Problem.h"

int main( int ,char**){
   Problem P = generateProblem(40,4,4,10,11);
  /* std::vector<int> qualif(3,0);
  qualif={1,1,1};
  Family F1(7,4,16,qualif); 
  qualif={0,1,0};
  Family F2(8,2,15,qualif);  
  qualif={1,1,1};
  Family F3(9,3,17,qualif);
  familyList F={F1,F2,F3};
  //creation du probleme
  qualif.resize(10);
  qualif={2,2,0,2,0,2,1,0,0,1};
  Problem P(10,3,qualif,F);*/
  
  std::cout << P.toString() << std::endl;
  std::cout << std::endl << "***********************************" << std::endl;
  std::cout << "Solving Problem with LH\n";
  std::cout  << "***********************************" << std::endl;
  Solution s1(P);  
  if (LH(P,s1)){
    std::cout << "Hurray!\n"; 
    std::cout << "Objective value:\t" << s1.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;
    std::cout << "valide? \t" << s1.isValid(P) << std::endl;
    std::cout << s1.toString() << std::endl; 
 
  }
  else  
    std::cout << "...\n";

  return 0;
}

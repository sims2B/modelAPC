#include "ListHeuristic.h"
#include "Problem.h"

int main( int ,char**){
   Problem P = generateProblem(20,5,4,10,13);
  /* std::vector<int> qualif(3,0);
  qualif[1]=1; qualif[0]=1;
  Family F1(7,2,14,qualif);  
  qualif[0]=0; qualif[2]=1;
  Family F2(6,2,21,qualif);  
  qualif[0]=1; qualif[2]=0;
  Family F3(8,2,18,qualif);
  familyList F={F1,F2,F3};
  //creation du probleme
  qualif.resize(10);
  qualif={2,0,2,2,0,2,2,2,1,0};
  Problem P(10,3,qualif,F);
  */
  std::cout << P.toString() << std::endl;
  std::cout << std::endl << "***********************************" << std::endl;
  std::cout << "Solving Problem with LH\n";
  std::cout  << "***********************************" << std::endl;
  Solution s1(P);  
  if (LH(P,s1)){
    std::cout << "Hurray!\n"; 
    std::cout << "Objective value:\t" << getObjectiveValue(P,s1,alpha,beta) << std::endl;
    std::cout << "valide? \t" << s1.isValid(P) << std::endl;
    std::cout << s1.toString() << std::endl; 
 
  }
  else  
    std::cout << "...\n";

  return 0;
}

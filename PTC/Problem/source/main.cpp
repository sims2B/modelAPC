#include "CPjobModel.h"
#include "jobModel.h"
#include "Problem.h"

int main( int ,char**){
  Problem P = generateProblem(20,3,2,10,4);
  /* std::vector<int> qualif(3,0);
  qualif[2]=1; qualif[0]=1;
  Family F1(6,3,16,qualif);  
  qualif[1]=1; qualif[2]=0;
  Family F2(8,3,6,qualif);
  Family F2(4,2,6,qualif);
  familyList F={F1,F2};
  //creation du probleme
  qualif.resize(6);
  qualif={1,0,1,0,1,0};
  Problem P(6,3,qualif,F);
  */
  std::cout << P.toString() << std::endl;
  std::cout << std::endl << "***********************************" << std::endl;
  /*  std::cout << "Solving Problem with MIP1\n";
  std::cout  << "***********************************" << std::endl;
  Solution s1(P);  
  if (!IP1::solve(P,s1)){
    std::cout << "Hurray!\n"; 
    std::cout << "valide? " << s1.isValid(P) << std::endl;
    std::cout << s1.toString() << std::endl; 
 
  }
  else  
    std::cout << "...\n";

  
  std::cout << std::endl << "***********************************" << std::endl;
  std::cout << "Solving Problem with CP\n";
  std::cout  << "***********************************" << std::endl;
    Solution s2(P);  
    if (!CP::solve(P,s2)){
    std::cout << "Hurray!\n"; 
    std::cout << "valide? " << s2.isValid(P) << std::endl; 
 
  }
  else  
    std::cout << "...\n";
  */
  return 0;
}

#include "QualifCentric.h"
#include "Problem.h"

int main( int ,char**){
  Problem P = generateProblem(20,2,4,10,6);
  /* std::vector<int> qualif(2,0);
  qualif = {1,1};
  Family F1(4,2,53,qualif);  
  qualif = {1,1};
  Family F2(9,2,60,qualif);  
  qualif = {0,1};
  Family F3(6,2,46,qualif);  
  qualif = {0,1};
  Family F4(4,2,32,qualif); 
  familyList F={F1,F2,F3,F4};
  //creation du probleme
  qualif.resize(20);
  qualif={1,0,0,3,1,0,0,3,0,0,0,2,1,1,0,1,2,0,0,0};
  Problem P(20,5,qualif,F);
  */
  
   std::cout << P.toString() << std::endl;
   std::cout << std::endl << "***********************************" << std::endl;
   std::cout << "Solving Problem with QCH\n";
   std::cout  << "***********************************" << std::endl;
   Solution s1(P);  
   if (QCH(P,s1)){
     std::cout << "Hurray!\n"; 
     std::cout << "Objective value:\t" << s1.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;
     std::cout << "valide? \t" << s1.isValid(P) << std::endl;
     std::cout << s1.toString() << std::endl; 
 
   }
   else  
     std::cout << "...\n";

   return 0;
}

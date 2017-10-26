#include "Recursive.h"
#include "Problem.h"
//#include "ListHeuristic.h"
//#include "SchedulingCentric.h"
#include "QualifCentric.h"

int main( int ,char**){
  Problem P = generateProblem(40,5,4,10,13);
  ptrHeur heuristic ;
  heuristic = QCH;
  /*std::vector<int> qualif(3,0);
  qualif = {1,0,1};
  Family F1(8,2,31,qualif);  
  qualif = {1,1,1};
  Family F2(9,2,42,qualif);  
  qualif = {0,1,1};
  Family F3(5,2,32,qualif);  
  familyList F={F1,F2,F3};
  //creation du probleme
  qualif.resize(10);
  qualif={0,0,0,0,0,2,1,0,0,0,1,2,0,1,2,1,2,2,0,2};
  Problem P(20,3,qualif,F);*/
  
   std::cout << P.toString() << std::endl;
   std::cout << std::endl << "***********************************" << std::endl;
   std::cout << "Solving Problem with LH\n";
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
   std::cout << std::endl << "***********************************" << std::endl;
   std::cout << "Solving Problem with QCH\n";
   std::cout  << "***********************************" << std::endl;
   Solution s2(P);  
   if (RH(P,s2,heuristic)){
     std::cout << "Hurray!\n"; 
     std::cout << "Objective value:\t" << s2.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;
     std::cout << "valide? \t" << s2.isValid(P) << std::endl;
     std::cout << s2.toString() << std::endl; 
 
   }
   else  
   std::cout << "...\n";

   return 0;
}

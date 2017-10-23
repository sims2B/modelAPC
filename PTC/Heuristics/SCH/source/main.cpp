#include "SchedulingCentric.h"
#include "Problem.h"

int main( int ,char**){
   Problem P = generateProblem(20,5,4,10,13);
   
   std::cout << P.toString() << std::endl;
   std::cout << std::endl << "***********************************" << std::endl;
   std::cout << "Solving Problem with SCH\n";
   std::cout  << "***********************************" << std::endl;
   Solution s1(P);  
   if (SCH(P,s1)){
     std::cout << "Hurray!\n"; 
     std::cout << "Objective value:\t" << s1.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;
     std::cout << "valide? \t" << s1.isValid(P) << std::endl;
     std::cout << s1.toString() << std::endl; 
 
   }
   else  
     std::cout << "...\n";

   return 0;
}

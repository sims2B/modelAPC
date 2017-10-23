#include "jobModel.h"
#include <iostream>
#include <fstream>
using namespace IP1;

int main(int,char **/*argv[]*/){
  //creation des familles
  std::vector<int> qualif(3,0);
  qualif[1]=1; qualif[0]=1;
  Family F1(3,2,4,qualif);  
  qualif[1]=0; qualif[2]=1;
  Family F2(4,2,5,qualif);
  familyList F={F1,F2};
  //creation du probleme
  qualif.resize(7);
  qualif={0,1,1,0,1,0,1};
  Problem P(7,3,qualif,F);

  /*int nbTask;
  std::ifstream instance(argv[1],std::ios::in);
  instance >> nbTask ;
  Problem P(nbTask,nbFam);
  P.readFromFile(instance);
  instance.close();
  */
  Solution s(P);  
  if (!solve(P,s)){
    std::cout << "Hurray!\n"; 
    std::cout << "valide? " << s.isValid(P) << std::endl; 
 
  }
  else  
    std::cout << "...\n";
  std::cout << "**************************************************" <<std::endl;

  return 0;
}

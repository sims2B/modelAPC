#include "CPjobModel.h"
#include <iostream>
#include <vector>
#include <fstream>

using namespace CP;

//typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
//typedef IloArray<IloIntervalSequenceVar> IloIntervalSequenceVarArray;


int main(int,char* argv[]){
 
	std::ifstream instance(argv[1], std::ios::in);
	Problem P = readFromFile(instance);
	instance.close();
	std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() << ";";
	Solution s(P);
	if (!solve(P, s))
		std::cout << ";" << s.isValid(P) << std::endl;
	else
		std::cout << time_limit << ";0; ; ; ; ; ; ; ; ; \n";

	//std::cout << P.toString() << s.toString();
  return 0;
}

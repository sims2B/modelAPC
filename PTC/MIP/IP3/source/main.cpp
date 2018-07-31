
#include "stdafx.h"
#include "improvedFamilyModel.h"


int main(int, char *argv[]){
	std::ifstream instance(argv[1], std::ios::in);
	Problem P = readFromFile(instance);
	instance.close();
	std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() << ";";
	Solution s(P);
	if (solve(P, s))
		std::cout << time_limit << ";0; ; ; ; ; ; ; \n";
	std::cout << P.toString() << s.toString(P);
	s.toTikz(P);
	return 0;
}
 
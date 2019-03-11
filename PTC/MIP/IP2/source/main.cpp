#include "familyModel.h"

int main(int,char *argv[]){
	std::ifstream instance(argv[1], std::ios::in);
	Problem P = readFromFile(instance);
	instance.close();
	std::cout << problem.N << ";" << problem.M << ";" << problem.getNbFams() << ";";
	Solution s(P);
	if (!solve(P, s))
		std::cout << ";" << s.isValid(P) << std::endl;
	else
		std::cout << time_limit << ";0; ; ; ; ; ; ; ; ; \n";

	//std::cout << problem.toString() << s.toString();
  return 0;

}

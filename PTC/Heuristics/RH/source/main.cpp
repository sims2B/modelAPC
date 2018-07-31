
#include "stdafx.h"
#include "Recursive.h"
#include "Problem.h"
//#include "ListHeuristic.h"
//#include "SchedulingCentric.h"
#include "QualifCentric.h"
using Clock = std::chrono::high_resolution_clock;

int main(int, char* argv[]){

	std::ifstream instance(argv[1], std::ios::in);
	Problem P = readFromFile(instance);
	instance.close();

	std::cout << P.N << ";" << P.M << ";" << P.getFamilyNumber() << ";";
	Solution s(P);

	ptrHeur heuristic;
	heuristic = QCH;

	Clock::time_point t1 = Clock::now();
	int solved = RH(P, s, heuristic);
	Clock::time_point t2 = Clock::now();
	std::chrono::duration<double> duration =
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	std::cout << duration.count() << ";";
	displayCVS(P, s, solved);
	if (solved) std::cout << ";" << s.isValid(P) << std::endl;
	else std::cout << "; \n";
	/*if (solved) {
		std::cout << P.toString() << s.toString();
		s.toTikz(P);
	}*/
	return 0; 
}

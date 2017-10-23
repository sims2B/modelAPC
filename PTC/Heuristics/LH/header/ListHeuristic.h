#include "Solution.h"
#include <vector>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

int LH(Problem P , Solution &s);
int chooseFamily(const Problem &P, int m, std::vector<int> toSchedule);
int treat(Problem &P, Solution& s, const int m, const int f, std::vector<int> &endLast,std::vector<int> &toSchedule, std::vector<int> &nextOfFam);

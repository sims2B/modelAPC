#include "Solution.h"
#include <vector>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

int SCH(Problem P , Solution &s);
int displayCVS(const Problem& P, const Solution& s, int& solved);

int chooseFamily(const Problem &P, const Solution& s, const int& m, const int& t, const int& current, std::vector<int> toSchedule);
//return the family index with the minimum remaining threshold on m at time t
int famWithMinThresh(const Problem &P, const Solution& s, const int &m, const int& t, std::vector<int> toSchedule);
//compute the remaining threshold of a family on m at time t
int remainingThresh(const Problem& P, const Solution& s, const int& f, const int& m, const int &t);
int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int> &endLast, std::vector<int> &toSchedule, std::vector<int> &nextOfFam);

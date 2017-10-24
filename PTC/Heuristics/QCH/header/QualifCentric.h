#include "Solution.h"
#include <vector>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

int QCH(const Problem &P , Solution &s);

/////////////// PHASE 1 //////////////////
int schedule(Problem P, Solution &s, std::vector<int>& endLast);
//return the family with the minimum remaining threshold
int chooseFamily(const Problem &P, const Solution& s, const int &m, const int& t, std::vector<int> toSchedule);
//compute the remaining threshold of a family on m at time t
int remainingThresh(const Problem& P, const Solution& s, const int& f, const int& m, const int &t);
int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int>& endLast, std::vector<int>& toSchedule, std::vector<int>& nextOfFam);


///////////// PHASE 2 //////////////////
int intraChange(const Problem &P, Solution& s, std::vector<int>& endLast);
int getLastOn(const Problem& P,const Solution& s,const int& k,
	      const int& t);
int addDisqualif(const Problem& P, const Solution& s, const int& j,const int& k);
int updateTime(const Problem& P, Solution& s, const int& i, const int& j, const int& k,
	       std::vector<int>& endLast);

#include "Solution.h"
#include <vector>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

int QCH(Problem P , Solution &s);

int displayCVS(const Problem& P, const Solution& s, bool& solved);
/////////////// PHASE 1 //////////////////
int schedule(Problem P, Solution &s, std::vector<int>& endLast);
//return the family with the minimum remaining threshold
int chooseFamily(const Problem &P, const Solution& s, const int &m, const int& t, std::vector<int> toSchedule);
//compute the remaining threshold of a family on m at time t
int remainingThresh(const Problem& P, const Solution& s, const int& f, const int& m, const int &t);
int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int>& endLast, std::vector<int>& toSchedule, std::vector<int>& nextOfFam);


///////////// PHASE 2 //////////////////
int intraChange(const Problem &P, Solution& s, std::vector<int>& endLast);

///////////// PHASE 3 /////////////////

int interChange(const Problem& P, Solution &s, std::vector<int>& endLast);
int addCompletion(const Problem& P, const int& i, const int& k, const int& m,
		  std::vector<int> endLast);

///////////// COMMON //////////////////


int getLastOn(const Problem& P,const Solution& s,const int& k,
	      const int& t);
int addDisqualif(const Problem& P, const Solution& s, const int& i, const int& m);
int updateTime(const Problem& P, Solution& s, const int& i, const int& j, const int& k,
	       const int& m , std::vector<int>& endLast);

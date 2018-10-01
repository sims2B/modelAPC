#include "Solution.h"
#include <vector>

#include <ctime>
#include <ratio>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;

namespace QualifCentric {
int displayCVS(const Problem&, const Solution&, int&);
int treat(Problem&, Solution&, const int, const int, std::vector<int>&, 
		std::vector<int>&, std::vector<int>&);
int remainingThresh(const Problem&, const Solution&, const int&, const int&, 
		const int&);
}

//remplacer addCompletion par reevaluation objectif?
int QCH(Problem P , Solution &s);
int QCHWithOutput(Problem P , Solution &s);

/////////////// PHASE 1 //////////////////
int schedule(Problem P, Solution &s, std::vector<int>& endLast);
//return the family with the minimum remaining threshold
int chooseFamily(const Problem &P, const Solution& s, const int &m, const int& t, 
		std::vector<int> toSchedule);

///////////// PHASE 2 //////////////////
int intraChange(const Problem &P, Solution& s, std::vector<int>& endLast);

///////////// PHASE 3 /////////////////

int interChange(const Problem& P, Solution &s, std::vector<int>& endLast);
int addCompletion(const Problem& P, const int& i, const int& nbJobs, const int& k, 
		const int& m, std::vector<int> endLast);
int getBeginOfLasts(const Problem& P, const Solution& s,  const int &last);
void findJobMachineMatch(const Problem& P, const Solution& s, int k, int j, 
		int firstOfLast, const std::vector<int>& endLast, int& machineSelected, 
		int& jobSelected);
///////////// COMMON //////////////////

//cherche sur la machine k le job qui finit à l'instant t (on l'utilise avec le completion 
// time de la machine pour avoir la derniere tache)
int getLastOn(const Problem& P,const Solution& s,const int& k,
		const int& t);
int addDisqualif(const Problem& P, const Solution& s, const int& i, const int &j, 
		const int& m, const int& k,const int& nbJobs);
int updateTime(const Problem& P, Solution& s, const int& i, const int& j, 
		const int& firstOfLast, const int& k, const int &m, 
		std::vector<int>& endLast);
void computeLastOccBefore(const Problem& P, const Solution& s, int m, int i, 
		std::vector<int>& lastOccBef);
void computeFirstOccAfter(const Problem& P, const Solution& s, int m, int i, 
		std::vector<int>& firstOccAfter);

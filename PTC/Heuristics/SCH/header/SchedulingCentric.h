#include "Solution.h"
#include <vector>


#include <ctime>
#include <ratio>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;


namespace SchedulingCentric {
int displayCVS(const Problem&, const Solution&, int&);
int treat(Problem&, Solution&, const int, const int, std::vector<int>&, 
		std::vector<int>&, std::vector<int>&);
int remainingThresh(const Problem&, const Solution&, const int&, const int&, 
					const int&);
}
		    
int SCH(Problem P , Solution &s);
int SCHWithOutput(Problem P , Solution &s);
int chooseFamily(const Problem &P, const Solution& s, const int& m, const int& t,
const int& current, std::vector<int> toSchedule);
//return the family getIndex with the minimum remaining threshold on m at time t
int famWithMinThresh(const Problem &P, const Solution& s, const int &m, 
const int& t, std::vector<int> toSchedule);


#include "Solution.h"
#include <algorithm>
#include <vector>
#include <tuple>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1// P.N*P.computeHorizon()
#endif

//parametre du recuit
#define TEMP_INIT 20000
#define COOLING_FACTOR 0.95
#define NB_TEMP_CHANGE 10000
#define TIME_LIMIT 600
//parametre incertains
#define ACCPET_RATE 1 // delta/kT ; k=1
#define NB_IT_PAR_TEMP 50 //???



#include <ctime>
#include <ratio>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
typedef int(*ptrHeur)(Problem,Solution&);

/////////////////////////////////////////////////////////////////////////////////////
//// ATTENTION!! SOLUTION TRIEE!!! (s ne doit pas forcément etre triée en entrée ////
/// mais est triée dans l'algo et les opérations doivent conservées ce tri!! ////////
/////////////////////////////////////////////////////////////////////////////////////

int SA(const Problem& P, Solution& s, ptrHeur&); 
//genere sol voisine de s dans currentSol , renvoie 1 si currentSol est valide, 0 sinon
int generateNeighborSol(const Problem& P, const Solution &s, Solution& currentSol); 

int intraChange(const Problem& P, const Solution& s, Solution& currentSol); 
void intraChangeMoveIBeforeJ(const Problem& P, const Solution& s, Solution& currentSol, int i, int j,int m);
void intraChangeMoveJBeforeI(const Problem& P, const Solution& s, Solution& currentSol, int i, int j,int m);

void startTimeUpdate(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int k);

int interChange(const Problem& P, const Solution& s, Solution& currentSol);
void interChangeUpdateMjThenMi(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj);
void interChangeUpdateMiThenMj(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj);

void acceptWithProba(Solution& s, const Solution& currentSol, const double currentTemp, const int delta);
namespace SAA{
	int displayCVS(const Problem& P, const Solution& s, int& solved);
}

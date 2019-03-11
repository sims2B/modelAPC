#include "SchedulingCentric.h"
#include "paramModelAPC.h"
#include <limits>
#include <vector>

using namespace SchedulingCentric;

namespace SchedulingCentric{
  int displayCVS(const Problem& problem, const Solution& s, int& solved){
    if (solved){
      std::cout << "1;";
      std::cout << s.getWeigthedObjectiveValue(problem) << ";"
		<< s.getSumCompletion(problem) << ";"
		<< s.getNbDisqualif() << ";" << s.getRealNbDisqualif(problem) << ";"
		<< s.getNbSetup(problem);
    }
    else
      std::cout << "0; ; ; ; ; ";
    return 0;
  }

  int treat(Problem &problem, Solution& s, const int m, const int f,
	    std::vector<int> &endLast, 
	    std::vector<int> &toSchedule,
	    std::vector<int> &nextOfFam){
    //remplissage de solution
    (s.qualifLostTime[f][m] == endLast[m] ? //if the last task on j is of family f
     s.S[nextOfFam[f]].start = endLast[m] ://no setup
     s.S[nextOfFam[f]].start = endLast[m] + problem.getSetup(f));//otw setup
    s.S[nextOfFam[f]].machine = m;
    s.S[nextOfFam[f]].index = nextOfFam[f];
    //update endLast
    endLast[m] = s.S[nextOfFam[f]].start + problem.getDuration(f);
    s.qualifLostTime[f][m] = endLast[m]; //update qualifLostTime
    toSchedule[f]--; //update nf (toSchedule)
    //update nextOfFam
    if (toSchedule[f] != 0) {
      nextOfFam[f]++;
      while (problem.famOf[nextOfFam[f]] != f && nextOfFam[f] < problem.N)
	nextOfFam[f]++;
    }
    //disqualification?
    for (int f2 = 0; f2 < problem.getNbFams(); ++f2){ //forall families(!= f and qualified on m)
      if (problem.F[f2].qualif[m] && f2 != f){
	if (s.qualifLostTime[f2][m] != 0){
	  if (s.qualifLostTime[f2][m] + problem.F[f2].threshold - problem.F[f2].duration < endLast[m] + problem.F[f2].setup){
	    s.qualifLostTime[f2][m] += problem.F[f2].threshold - problem.F[f2].duration;
	    problem.F[f2].qualif[m] = 0;
	  }
	}
	else{
	  if (s.qualifLostTime[f2][m] + problem.F[f2].threshold < endLast[m] + problem.F[f2].setup){
	    s.qualifLostTime[f2][m] += problem.F[f2].threshold;
	    problem.F[f2].qualif[m] = 0;
	  }
	}
      }
    }
    return 0;
  }

  int remainingThresh(const Problem& problem, const Solution& s, const int& f, const int& m,
		      const int &t){
    return s.qualifLostTime[f][m] + problem.getThreshold(f) - t;
  }

}

int SCHWithOutput(Problem problem, Solution& s){
  Clock::time_point t1 = Clock::now();
  int solved = SCH(problem,s);
  Clock::time_point t2 = Clock::now();
  std::chrono::duration<double> duration =
    std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
  std::cout << duration.count() << ";";
  displayCVS(problem, s, solved);
  if (solved)
    std::cout << ";" << s.isValid(problem) << std::endl;
  else std::cout << "; \n";

  return 0;
}

int SCH(Problem problem, Solution& s){
  const int F = problem.getNbFams();
  int i, f, j;
  std::vector<int> endLast(problem.M, 0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F, 0); //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);
  std::vector<int> lastFam(problem.M, -1); //next i s.t. fam(i) == f
  for (f = 0; f < F; ++f) toSchedule[f] = problem.getNf(f);
  for (f = 0; f < F; ++f){
    i = 0;
    while (i < problem.N && problem.famOf[i] != f) ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine. 
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
      if (problem.F[f].qualif[i])
	s.qualifLostTime[f][i] = 0;
  i = 0;
  while (i < problem.N){
    bool feasible = false;

    for (j = 0; j < problem.M; ++j){
      if ((f = chooseFamily(problem, s, j, endLast[j], lastFam[j], toSchedule)) != -1){
	feasible = true;
	lastFam[j] = f;
	treat(problem, s, j, f, endLast, toSchedule, nextOfFam);
	++i;
      }

    }

    if (!feasible) return 0;
  }
  int Cmax = s.getMaxEnd(problem);
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i){
      if (problem.F[f].qualif[i]){
	if (std::max(s.qualifLostTime[f][i] - problem.getDuration(f), 0) + problem.getThreshold(f) < Cmax)
	  (s.qualifLostTime[f][i] == 0 ? s.qualifLostTime[f][i] += problem.getThreshold(f) : s.qualifLostTime[f][i] += problem.getThreshold(f) - problem.getDuration(f));
	else s.qualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
      else{
	if (s.qualifLostTime[f][i] >= Cmax)
	  s.qualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
    }
  return 1;
}


int famWithMinThresh(const Problem &problem, const Solution& s, const int &m,
		     const int& t, std::vector<int> toSchedule){
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f){
    if (problem.F[f].qualif[m] && toSchedule[f] != 0){
      if (selected == -1) selected = f;
      else if (remainingThresh(problem, s, f, m, t) < remainingThresh(problem, s, selected, m, t))
	selected = f;
      else if (remainingThresh(problem, s, f, m, t) == remainingThresh(problem, s, selected, m, t)
	       && problem.getDuration(f) < problem.F[selected].duration)
	selected = f;
    }
  }
  return selected;
}

int chooseFamily(const Problem &problem, const Solution& s, const int& m, const int& t,
		 const int& current, std::vector<int> toSchedule){
  int critical, selected = -1;
  //if there's no task on m, choose with the minimum threshold rule
  if (current == -1)
    selected = famWithMinThresh(problem, s, m, 0, toSchedule);
  else {
    //if there's no task of current left choose with the minimum remaining threshold rule
    if (toSchedule[current] == 0)
      return famWithMinThresh(problem, s, m, t, toSchedule);
    else {
      selected = current;
      //select family with min remaining threshold 
      if ((critical = famWithMinThresh(problem, s, m, t, toSchedule)) != -1)
	if (remainingThresh(problem, s, critical, m, t) - problem.F[critical].setup < problem.F[selected].duration)
	  selected = critical;
    }
  }
  return selected;
}

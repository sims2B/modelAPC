#include "ListHeuristic.h"
#include <limits>
//#include "utils.h"



int LH(Problem problem, Solution& s){
  //init and variable
  const int F = problem.getNbFams();
  int i ,f;
  std::vector<int> endLast(problem.M,0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F,0); //number of task to schedule in f
  std::vector<int> nextOfFam(F,-1); //next i s.t. fam(i) == f
  for (f = 0 ; f < F ; ++f) toSchedule[f] = problem.getNf(f);
  for (f = 0 ; f < F ; ++f){
    i=0;
    while (i < problem.N && problem.famOf[i]!=f) ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine. 
  for ( f = 0 ; f < F ; ++f)
    for (i = 0 ; i < problem.M ; ++i)
      if (problem.F[f].qualif[i])
	s.qualifLostTime[f][i] = 0;


  //algo
  i = 0;
  while (i < problem.N){
    bool feasible = false;
    for (int j = 0 ; j < problem.M ; ++j){
      if ((f = chooseFamily(problem,j,toSchedule)) != -1 ){
	treat(problem,s,j,f,endLast,toSchedule,nextOfFam);
	feasible = true;
	i++;
      }
    }
    if (feasible == false) return 0;
  }
  for (f = 0 ; f < F ; ++f)
    for (i = 0 ; i < problem.M ; ++i)
      if (problem.F[f].qualif[i] || endLast[i] <= s.qualifLostTime[f][i]) s.qualifLostTime[f][i] = std::numeric_limits<int>::max(); 
  return 1;
}

int displayCVS(const Problem& P, const Solution& s, const int& solved){
  if (solved){
    std::cout << "1;";
    std::cout << s.getWeigthedObjectiveValue(P) << ";"
	      << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P) ;
  }
  else
    std::cout << "0; ; ; ; ; ";
  return 0;
}


int treat(Problem &problem,Solution& s,const int m, const int f, std::vector<int> &endLast,std::vector<int> &toSchedule, std::vector<int> &nextOfFam){
  //remplissage de solution
  (s.qualifLostTime[f][m] == endLast[m]? //if the last task on j is of family f
   s.S[nextOfFam[f]].start = endLast[m] ://no setup
   s.S[nextOfFam[f]].start = endLast[m] + problem.getSetup(f));//otw setup
  s.S[nextOfFam[f]].machine = m;
  s.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = s.S[nextOfFam[f]].start + problem.getDuration(f);
  s.qualifLostTime[f][m] = endLast[m]; //update qualifLostTime
  toSchedule[f]--; //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] !=0) {
    nextOfFam[f]++;
    while (problem.famOf[nextOfFam[f]] != f && nextOfFam[f] < problem.N)
      nextOfFam[f]++;
  }
  //disqualification?
  for (int f2 = 0 ; f2 < problem.getNbFams() ; ++f2){ //forall families(!= f and qualified on m)
    if (problem.F[f2].qualif[m] && f2!=f)
      if (s.qualifLostTime[f2][m] + problem.F[f2].threshold - problem.F[f2].duration  < endLast[m] + problem.F[f2].setup){
		  (s.qualifLostTime[f2][m] == 0 ? s.qualifLostTime[f2][m] += problem.F[f2].threshold : s.qualifLostTime[f2][m] += problem.F[f2].threshold - problem.F[f2].duration);
	problem.F[f2].qualif[m]=0;
      }
  }
  return 0;
}

int chooseFamily(const Problem &problem, int m, std::vector<int> toSchedule){
  int selected = -1;
  for (int f = 0 ; f < problem.getNbFams() ; ++f){
    if (problem.F[f].qualif[m] && toSchedule[f] != 0){
      if (selected == -1) selected = f;
      else if (problem.getThreshold(f) < problem.F[selected].threshold)
	selected = f;
      else if (problem.getThreshold(f) == problem.F[selected].threshold
	       && problem.getDuration(f) < problem.F[selected].duration)
	selected = f;
    }
  }
  return selected;
}

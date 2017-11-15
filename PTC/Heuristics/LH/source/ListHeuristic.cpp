#include "ListHeuristic.h"
#include "utils.h"
#include <limits>
#include <vector>



int LH(Problem P, Solution& s){
  //init and variable
  const int F = P.getFamilyNumber();
  int i ,f;
  std::vector<int> endLast(P.M,0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F,0); //number of task to schedule in f
  std::vector<int> nextOfFam(F,-1); //next i s.t. fam(i) == f
  for (f = 0 ; f < F ; ++f) toSchedule[f] = P.getNf(f);
  for (f = 0 ; f < F ; ++f){
    i=0;
    while (i < P.N && P.famOf[i]!=f) ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine. 
  for ( f = 0 ; f < F ; ++f)
    for (i = 0 ; i < P.M ; ++i)
      if (P.F[f].qualif[i])
	s.QualifLostTime[f][i]=0;


  //algo
  i = 0;
  while (i < P.N){
    bool feasible = false;
    for (int j = 0 ; j < P.M ; ++j){
      if ((f = chooseFamily(P,j,toSchedule)) != -1 ){
	treat(P,s,j,f,endLast,toSchedule,nextOfFam);
	feasible = true;
	i++;
      }
    }
    if (feasible == false) return 0;
  }
  for (f = 0 ; f < F ; ++f)
    for (i = 0 ; i < P.M ; ++i)
      if (P.F[f].qualif[i] || endLast[i] <= s.QualifLostTime[f][i]) s.QualifLostTime[f][i] = std::numeric_limits<int>::max(); 
  return 1;
}

int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int> &endLast,std::vector<int> &toSchedule, std::vector<int> &nextOfFam){
  //remplissage de solution
  (s.QualifLostTime[f][m] == endLast[m]? //if the last task on j is of family f
   s.S[nextOfFam[f]].start = endLast[m] ://no setup
   s.S[nextOfFam[f]].start = endLast[m] + P.F[f].setup);//otw setup
  s.S[nextOfFam[f]].machine = m;
  s.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = s.S[nextOfFam[f]].start + P.F[f].duration;
  s.QualifLostTime[f][m] = endLast[m]; //update QualifLostTIme
  toSchedule[f]--; //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] !=0) {
    nextOfFam[f]++;
    while (P.famOf[nextOfFam[f]] != f && nextOfFam[f] < P.N)
      nextOfFam[f]++;
  }
  //disqualification?
  for (int f2 = 0 ; f2 < P.getFamilyNumber() ; ++f2){ //forall families(!= f and qualified on m)
    if (P.F[f2].qualif[m] && f2!=f)
      if (s.QualifLostTime[f2][m] + P.F[f2].threshold  < endLast[m] + P.F[f2].setup){
	s.QualifLostTime[f2][m] +=  P.F[f2].threshold;
	P.F[f2].qualif[m]=0;
      }
  }
  return 0;
}

int chooseFamily(const Problem &P, int m, std::vector<int> toSchedule){
  int selected = -1;
  for (int f = 0 ; f < P.getFamilyNumber() ; ++f){
    if (P.F[f].qualif[m] && toSchedule[f] != 0){
      if (selected == -1) selected = f;
      else if (P.F[f].threshold < P.F[selected].threshold)
	selected = f;
      else if (P.F[f].threshold == P.F[selected].threshold
	       && P.F[f].duration < P.F[selected].duration)
	selected = f;
    }
  }
  return selected;
}

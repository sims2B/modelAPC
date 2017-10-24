#include "SchedulingCentric.h"
#include <limits>
#include <vector>



int SCH(Problem P, Solution& s){
  const int F = P.getFamilyNumber();
  int i ,f , j;
  std::vector<int> endLast(P.M,0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F,0); //number of task to schedule in f
  std::vector<int> nextOfFam(F,-1);
  std::vector<int> lastFam(P.M,-1); //next i s.t. fam(i) == f
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
  i=0;
  while (i < P.N){
    bool feasible = false;

    for (j = 0 ;j < P.M ; ++j){
      if ((f = chooseFamily(P,s,j,endLast[j],lastFam[j],toSchedule)) != -1 ){
	feasible = true;
	lastFam[j] = f;
	treat(P,s,j,f,endLast,toSchedule,nextOfFam);
	++i;
      }
	
    }
    
    if (!feasible) return 0;
  }
  for (f = 0 ; f < F ; ++f)
    for (i = 0 ; i < P.M ; ++i)
      if (P.F[f].qualif[i] || endLast[i] <= s.QualifLostTime[f][i]) s.QualifLostTime[f][i] = std::numeric_limits<int>::max(); 
  return 1;
}


int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int> &endLast, std::vector<int> &toSchedule, std::vector<int> &nextOfFam){
  //remplissage de solution
  (s.QualifLostTime[f][m] == endLast[m]? //if the last task on j is of family f
   s.S[nextOfFam[f]].start = endLast[m] ://no setup
   s.S[nextOfFam[f]].start = endLast[m] + P.F[f].setup);//otw setup
  s.S[nextOfFam[f]].machine = m;
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
 
int remainingThresh(const Problem& P, const Solution& s, const int& f, const int& m,
		    const int &t){
  return s.QualifLostTime[f][m] + P.F[f].threshold - t;
}

int famWithMinThresh(const Problem &P, const Solution& s, const int &m,
		     const int& t, std::vector<int> toSchedule){
  int selected = -1;
  for (int f = 0 ; f < P.getFamilyNumber() ; ++f){
    if (P.F[f].qualif[m] && toSchedule[f] != 0){
      if (selected == -1) selected = f;
      else if (remainingThresh(P,s,f,m,t) < remainingThresh(P,s,selected,m,t))
	selected = f;
      else if (remainingThresh(P,s,f,m,t) == remainingThresh(P,s,selected,m,t)
	       && P.F[f].duration < P.F[selected].duration)
	selected = f;
    }
  }
  return selected;
}

int chooseFamily(const Problem &P, const Solution& s, const int& m, const int& t,
		 const int& current, std::vector<int> toSchedule){
  int critical , selected = -1;
  //if there's no task on m, choose with the minimum threshold rule
  if (current == -1)
    selected = famWithMinThresh(P,s,m,0,toSchedule);
  else {
    //if there's no task of current left choose with the minimum remaining threshold rule
    if (toSchedule[current] == 0)
      return famWithMinThresh(P,s,m,t,toSchedule);
    else {
      selected = current;
      //select family with min remaining threshold 
      if ((critical = famWithMinThresh(P,s,m,t,toSchedule))!= -1)
	if (remainingThresh(P,s,critical,m,t) - P.F[critical].setup < P.F[selected].duration)
	  selected = critical;
    }
  }
  return selected;
}

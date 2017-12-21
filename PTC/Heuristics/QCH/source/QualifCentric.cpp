#include "QualifCentric.h"
#include "utils.h"
#include <limits>
#include <vector>
#include <cstdlib>



int QCH(Problem P, Solution& s){
  std::vector<int> endLast(P.M,0); //end time of the last task scheduled of m
  if (schedule(P,s,endLast)){//phase 1
    /*   std::cout << s.toString() << std::endl;
    std::cout << "Phase 1 valide ? " << s.isValid(P) << std::endl;
    std::cout << "Objective value before phase 2:\t" <<
    s.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;*/
    if (!intraChange(P,s,endLast)){
      /* std::cout << s.toString() << std::endl;
    std::cout << "Phase 2 valide ? " << s.isValid(P) << std::endl;
    std::cout << "Objective value before phase 3:\t" <<
    s.getWeigthedObjectiveValue(P,alpha,beta) << std::endl;*/
      return !interChange(P,s,endLast);
    }
    else {
      std:: cout << "Problem in Phase 2\n"; return 0;
    }
  }
  else {
    std::cout << "No solution Found\n"; return 0;
  }
}


int displayCVS(const Problem& P, const Solution& s, bool& solved){
  if (solved){
    std::cout << "1;";
    std::cout << s.getWeigthedObjectiveValue(P,alpha,beta) << ";"
	      << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P) ;
  }
  else
    std::cout << "0; ; ; ; ; ";
  return 0;
}

int schedule(Problem P, Solution& s, std::vector<int>& endLast){
  const int F = P.getFamilyNumber();
  int i ,f , j;
  std::vector<int> toSchedule(F,0); //number of task to schedule in f
  std::vector<int> nextOfFam(F,-1);
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
      if ((f = chooseFamily(P,s,j,endLast[j],toSchedule)) != -1 ){
	feasible = true;
	treat(P,s,j,f,endLast,toSchedule,nextOfFam);
	++i;
      }
    }
    
    if (!feasible) return 0;
  }
  for (f = 0 ; f < F ; ++f)
    for (i = 0 ; i < P.M ; ++i)
      if (P.F[f].qualif[i] || endLast[i] <=  s.QualifLostTime[f][i]) s.QualifLostTime[f][i] = std::numeric_limits<int>::max(); 
  return 1;
}

int treat(Problem &P,Solution& s,const int m, const int f, std::vector<int>& endLast, std::vector<int>& toSchedule, std::vector<int>& nextOfFam){
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
 
int remainingThresh(const Problem& P, const Solution& s, const int& f, const int& m,
		    const int &t){
  return s.QualifLostTime[f][m] + P.F[f].threshold - t;
}

int chooseFamily(const Problem &P, const Solution& s, const int &m, const int& t,
		 std::vector<int> toSchedule){
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

int intraChange(const Problem &P, Solution& s, std::vector<int>& endLast){
  int k,j,i = 0;
  const int F = P.getFamilyNumber();
  for (k = 0 ; k < P.M ; ++k){
    std::vector<int> moved(P.N,0);
    std::vector<int> firstOcc(F,-1);
    i=0;
    while (i < P.N){
      if (s.S[i].machine == k &&
	  (firstOcc[P.famOf[i]] == -1 ||
	   s.S[firstOcc[P.famOf[i]]].start + P.getDuration(firstOcc[P.famOf[i]])
	   == s.S[i].start))  
	firstOcc[P.famOf[i]] = i;
      ++i;
    }
    bool update ;
    do {
      update = false;
      j = getLastOn(P,s,k,endLast[k]);
      i = firstOcc[P.famOf[j]]; 
      if (i != -1 && i != j && !moved[j]  && !addDisqualif(P,s,i,k)){
        update = true;
	moved[j]=1;
	updateTime(P,s,i,j,k,k,endLast);
      }
    } while (update);
  }
  return 0;
}


int interChange(const Problem& P, Solution &s, std::vector<int>& endLast){
  int k , m , i , j;
  std::vector<int> moved(P.N,0);
  for (k = 0 ; k < P.M ; ++k){
    bool update ;
    do {
      update = false;
      if ((j = getLastOn(P,s,k,endLast[k]))!=P.N){
	for ( m = 0 ; m < P.M ; ++m){
	  if (P.isQualif(j,m) && k != m){
	    i = 0;
	    while ( i < P.N ){
	      if (P.isQualif(i,m) && s.S[i].machine == m && P.famOf[i]==P.famOf[j]) {
		if (i < P.N && !moved[j]  && !addDisqualif(P,s,i,m)
		    && !addCompletion(P,i,k,m,endLast)){
		  update = true;
		  moved[j]=1;
		  updateTime(P,s,i,j,k,m,endLast);
		  j = getLastOn(P,s,k,endLast[k]);
		  i = P.N; //break while
		}
	      }
	      ++i;
	    }
	  }
	}
      }
    }while (update);
  }
  return 0;
}


int addCompletion(const Problem& P, const int& i, const int& k, const int& m, std::vector<int> endLast){
  return (endLast[k]  <= endLast[m] + P.getDuration(i));
}

int getLastOn(const Problem& P, const Solution& s, const int& k,
	      const int& t){
  int i = 0;
  while (i < P.N && (s.S[i].machine != k || s.S[i].start + P.getDuration(i) != t) )
    ++i;
  return i;
}

// i sur machine m ; j sur machine k
int addDisqualif(const Problem& P, const Solution& s, const int& i, const int& m){
  int cur;
  const int F = P.getFamilyNumber();
  std::vector<int> firstOccAfter(F,P.N);
  std::vector<int> lastOccBef(F,-1);
  //compute last occurence of each familiy before i on k
  for (cur = 0 ; cur < P.N ; ++cur)
    if ( s.S[cur].machine == m && s.S[cur].start < s.S[i].start){
      if (lastOccBef[P.famOf[cur]] == -1)
	lastOccBef[P.famOf[cur]] = cur;
      else if (s.S[lastOccBef[P.famOf[cur]]].start < s.S[cur].start )
	lastOccBef[P.famOf[cur]] = cur;
    }
  //compute first occurence of each familiy after i on k
  for (cur = 0 ; cur < P.N ; ++cur)
    if ( s.S[cur].machine == m && s.S[cur].start > s.S[i].start){
      if (firstOccAfter[P.famOf[cur]] == P.N)
      firstOccAfter[P.famOf[cur]] = cur;
      else if (s.S[firstOccAfter[P.famOf[cur]]].start > s.S[cur].start )
	firstOccAfter[P.famOf[cur]] = cur;
    }
  //we just have two check if the "qualification" still holds between the last occ
  //of f before i and the first one after i
  for (int f = 0 ; f < F ; ++f){
    if (f != P.famOf[i] && P.F[f].qualif[m]){
      if (lastOccBef[f] == -1){
	if (firstOccAfter[f] < P.N)
	  if (s.S[firstOccAfter[f]].start + P.getDuration(i) > P.F[f].threshold)
	    return 1;
      }
      else	
	if (firstOccAfter[f] < P.N)
	  if (s.S[firstOccAfter[f]].start + P.getDuration(i)  -
	      (s.S[lastOccBef[f]].start + P.F[f].duration) > P.F[f].threshold)
	    return 1;
    }
  }
  return 0;
}

int updateTime(const Problem& P, Solution& s, const int& i, const int& j,
	       const int& k, const int &m, std::vector<int>& endLast){
  int cur = 0;
  std::vector<int> update(P.getFamilyNumber(),0);
  s.S[j].start = s.S[i].start + P.getDuration(i);
  s.S[j].machine = m;
  endLast[m] += P.getDuration(j);
  endLast[k] =  0;
  while (cur < P.N){
    // shift all the task after j by p_j on m
    if (cur != j && s.S[cur].machine == m && s.S[cur].start >  s.S[i].start){
      s.S[cur].start += P.getDuration(j);
      if (s.QualifLostTime[P.famOf[cur]][m] < std::numeric_limits<int>::max() && !update[P.famOf[cur]]){
	update[P.famOf[cur]]=1;
	s.QualifLostTime[P.famOf[cur]][m] += P.getDuration(j);
      }
    }
    //update endLast on k
    if (s.S[cur].machine == k && endLast[k] < s.S[cur].start + P.getDuration(cur))
      endLast[k] = s.S[cur].start + P.getDuration(cur);
    ++cur;

  }
  return 0;
}

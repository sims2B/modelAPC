#include "QualifCentric.h"
#include <limits>
#include <algorithm>

int QCH(Problem P, Solution& s){
  std::vector<int> endLast(P.M, 0);
  if (schedule(P, s, endLast)){
    if (!intraChange(P, s, endLast))
      return !interChange(P, s, endLast);
    else return 0;
    return 1;
  }
  else return 0;
}


int displayCVS(const Problem& P, const Solution& s, int& solved){
  if (solved){
    std::cout << "1;";
    std::cout << s.getWeigthedObjectiveValue(P, alpha_C, beta_Y) << ";"
	      << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P);
  }
  else
    std::cout << "0; ; ; ; ; ";
  return 0;
}

int schedule(Problem P, Solution& s, std::vector<int>& endLast){
  const int F = P.getFamilyNumber();
  int i, f, j;
  std::vector<int> toSchedule(F, 0); //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);
  for (f = 0; f < F; ++f) toSchedule[f] = P.getNf(f);
  for (f = 0; f < F; ++f){
    i = 0;
    while (i < P.N && P.famOf[i] != f) ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine. 
  for (f = 0; f < F; ++f)
    for (i = 0; i < P.M; ++i)
      if (P.F[f].qualif[i])
	s.QualifLostTime[f][i] = 0;


  i = 0;
  while (i < P.N){
    bool feasible = false;
    for (j = 0; j < P.M; ++j){
      if ((f = chooseFamily(P, s, j, endLast[j], toSchedule)) != -1){
	feasible = true;
	treat(P, s, j, f, endLast, toSchedule, nextOfFam);
	++i;
      }
    }

    if (!feasible) return 0;
  }

  int Cmax = s.getMaxEnd(P);
  for (f = 0; f < F; ++f)
    for (i = 0; i < P.M; ++i){
      if (P.F[f].qualif[i]){
	if (std::max(s.QualifLostTime[f][i] - P.F[f].duration, 0) + P.F[f].threshold < Cmax)
	  (s.QualifLostTime[f][i] == 0 ? s.QualifLostTime[f][i] += P.F[f].threshold : s.QualifLostTime[f][i] += P.F[f].threshold - P.F[f].duration);
	else s.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
      else{
	if (s.QualifLostTime[f][i] >= Cmax)
	  s.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
    }
  return 1;
}

int treat(Problem &P, Solution& s, const int m, const int f, std::vector<int>& endLast, std::vector<int>& toSchedule, std::vector<int>& nextOfFam){
  //remplissage de solution
  (s.QualifLostTime[f][m] == endLast[m] ? //if the last task on j is of family f
   s.S[nextOfFam[f]].start = endLast[m] ://no setup
   s.S[nextOfFam[f]].start = endLast[m] + P.F[f].setup);//otw setup
  s.S[nextOfFam[f]].machine = m;
  s.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = s.S[nextOfFam[f]].start + P.F[f].duration;
  s.QualifLostTime[f][m] = endLast[m]; //update QualifLostTIme
  toSchedule[f]--; //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] != 0) {
    nextOfFam[f]++;
    while (P.famOf[nextOfFam[f]] != f && nextOfFam[f] < P.N)
      nextOfFam[f]++;
  }
  //disqualification?	
  for (int f2 = 0; f2 < P.getFamilyNumber(); ++f2){ //forall families(!= f and qualified on m)
    if (P.F[f2].qualif[m] && f2 != f){
      if (s.QualifLostTime[f2][m] != 0){
	if (s.QualifLostTime[f2][m] + P.F[f2].threshold - P.F[f2].duration < endLast[m] + P.F[f2].setup){
	  s.QualifLostTime[f2][m] += P.F[f2].threshold - P.F[f2].duration;
	  P.F[f2].qualif[m] = 0;
	}
      }
      else{
	if (s.QualifLostTime[f2][m] + P.F[f2].threshold < endLast[m] + P.F[f2].setup){
	  s.QualifLostTime[f2][m] += P.F[f2].threshold;
	  P.F[f2].qualif[m] = 0;
	}
      }
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
  for (int f = 0; f < P.getFamilyNumber(); ++f){
    if (P.F[f].qualif[m] && toSchedule[f] != 0){
      if (selected == -1) selected = f;
      else if (remainingThresh(P, s, f, m, t) < remainingThresh(P, s, selected, m, t))
	selected = f;
      else if (remainingThresh(P, s, f, m, t) == remainingThresh(P, s, selected, m, t)
	       && P.F[f].duration < P.F[selected].duration)
	selected = f;
    }
  }
  return selected;
}

int intraChange(const Problem &P, Solution& s, std::vector<int>& endLast){
  int k, j, i = 0;
  const int F = P.getFamilyNumber();
  for (k = 0; k < P.M; ++k){ //sur chaque machine
    if (s.getNbJobsOn(k) > 2){
      std::vector<int> moved(P.N, 0);
      std::vector<int> firstOcc(F, -1);
      i = 0;
      //compute firstOcc of each family
      while (i < P.N){
	if (s.S[i].machine == k &&
	    (firstOcc[P.famOf[i]] == -1 ||
	     s.S[firstOcc[P.famOf[i]]].start + P.getDuration(firstOcc[P.famOf[i]])
	     == s.S[i].start))
	  firstOcc[P.famOf[s.S[i].index]] = i;
	++i;
      }
      //intraChange movement
      bool update;
      do {
	update = false;
	j = getLastOn(P, s, k, endLast[k]); //recupere l'index du dernier élément
	i = firstOcc[P.famOf[s.S[j].index]];
	if (i != -1 && i != j && !moved[s.S[j].index] && !addDisqualif(P, s, i, j, k, k, 1)){
	  update = true;
	  moved[s.S[j].index] = 1;
	  updateTime(P, s, i, j, j, k, k, endLast);
	  firstOcc[P.famOf[s.S[j].index]] = j;
	}
      } while (update);
    }
    int Cmax = s.getMaxEnd(P);
    for (int f = 0; f < P.getFamilyNumber(); ++f)
      for (i = 0; i < P.M; ++i){
	if (s.QualifLostTime[f][i] >= Cmax) s.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }
  return 0;
}

int interChange(const Problem& P, Solution &s, std::vector<int>& endLast){
  int k, m, i, j;
  std::sort(s.S.begin(), s.S.end());
  //std::vector<int> moved(P.N, 0);
  for (k = 0; k < P.M; ++k){
    if (s.getNbJobsOn(k) > 1){// pour chaque machine
      bool update;
      do {// tant qu'on fait un move
	update = false;
	// recuperer la derniere tache sur k (si il y en a une)
	if ((j = getLastOn(P, s, k, endLast[k])) != P.N){
	  int firstOfLast = getBeginOfLasts(P, s, k, j);
	  int machineSelected = -1;
	  for (m = 0; m < P.M; ++m){
	    if (P.isQualif(s.S[j].index, m) && k != m){// sur quelle machine je la met...
	      i = 0;
	      while (i < P.N){
		if (s.S[i].machine == m && P.famOf[s.S[i].index] == P.famOf[s.S[j].index]) { //...et avec quelle tache
		  if (!addDisqualif(P, s, i, firstOfLast, m, k, j - firstOfLast + 1) && !addCompletion(P, s.S[i].index, j - firstOfLast + 1, k, m, endLast)){
		    ((machineSelected == -1 || endLast[m] < endLast[machineSelected]) ?
		     machineSelected = m : machineSelected = machineSelected);
		    i = P.N; //break while
		  }
		}
		++i;
	      }
	    }
	  }
	  if (machineSelected != - 1) {
	    update = true;
	    //moved[s.S[j].index] = 1;
	    updateTime(P, s, i, j, firstOfLast, k, machineSelected, endLast);
	    std::sort(s.S.begin(), s.S.end());
	  }
	}
      } while (update);
    }

    int Cmax = s.getMaxEnd(P);
    for (int f = 0; f < P.getFamilyNumber(); ++f)
      for (i = 0; i < P.M; ++i){
	if (s.QualifLostTime[f][i] >= Cmax) s.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }
  return 0;
}

int addCompletion(const Problem& P, const int& i, const int& nbJobs, const int& k, const int& m, std::vector<int> endLast){
  return (endLast[k] <= endLast[m] + nbJobs * P.getDuration(i));
}

int getLastOn(const Problem& P, const Solution& s, const int& k, const int& t){
  int i = 0;
  while (i < P.N && (s.S[i].machine != k || s.S[i].start + P.getDuration(s.S[i].index) != t))
    ++i;
  return i;
}

int getBeginOfLasts(const Problem& P, const Solution& s, const int& k, const int &last){
  int i = last;
  while (i >= 0 && P.famOf[s.S[i].index] == P.famOf[s.S[last].index])
    --i;
  return i + 1;
}

// i sur machine m ; j sur machine k
int addDisqualif(const Problem& P, const Solution& s, const int& i, const int &j, const int& m, const int & k, const int& nbJobs){
  int cur;
  const int F = P.getFamilyNumber();
  std::vector<int> firstOccAfter(F, P.N);
  std::vector<int> lastOccBef(F, -1);
  //adding j on m add a disqualif? 

  //compute last occurence of each familiy before i on m
  for (cur = 0; cur < P.N; ++cur)
    if (s.S[cur].machine == m && s.S[cur].start < s.S[i].start){
      if (lastOccBef[P.famOf[s.S[cur].index]] == -1)
	lastOccBef[P.famOf[s.S[cur].index]] = cur;
      else if (s.S[lastOccBef[P.famOf[s.S[cur].index]]].start < s.S[cur].start)
	lastOccBef[P.famOf[s.S[cur].index]] = cur;
    }
  //compute first occurence of each familiy after i on m
  for (cur = 0; cur < P.N; ++cur){
    if (s.S[cur].machine == m && s.S[cur].start > s.S[i].start){
      if (firstOccAfter[P.famOf[s.S[cur].index]] == P.N)
	firstOccAfter[P.famOf[s.S[cur].index]] = cur;
      else if (s.S[firstOccAfter[P.famOf[s.S[cur].index]]].start > s.S[cur].start)
	firstOccAfter[P.famOf[s.S[cur].index]] = cur;
    }
  }
  //we just have two check if the "qualification" still holds between the last occ
  //of f before i and the first one after i
  for (int f = 0; f < F; ++f){
    if (f != P.famOf[s.S[i].index] && P.F[f].qualif[m]){
      if (lastOccBef[f] == -1){
	if (firstOccAfter[f] < P.N)
	  if (s.S[firstOccAfter[f]].start + nbJobs * P.getDuration(s.S[i].index) > P.F[f].threshold)
	    return 1;
      }
      else
	if (firstOccAfter[f] < P.N)
	  if (s.S[firstOccAfter[f]].start + nbJobs * P.getDuration(s.S[i].index) -
	      s.S[lastOccBef[f]].start  > P.F[f].threshold)
	    return 1;
    }
  }
  //removing j on k add a disqualif?
  for (cur = 0; cur < P.N; ++cur){
    if (j != cur && P.famOf[s.S[j].index] == P.famOf[s.S[cur].index] && s.S[cur].machine == k && s.S[cur].start < s.S[j].start){
      if (lastOccBef[P.famOf[s.S[cur].index]] == -1)
	lastOccBef[P.famOf[s.S[cur].index]] = cur;
      else if (s.S[lastOccBef[P.famOf[s.S[cur].index]]].start < s.S[cur].start)
	lastOccBef[P.famOf[s.S[cur].index]] = cur;
    }
  }
  if (lastOccBef[P.famOf[s.S[j].index]] == -1){
    if (P.F[P.famOf[s.S[j].index]].threshold > s.getMaxEnd(P))
      return 1;
  }
  else if (s.S[lastOccBef[P.famOf[s.S[j].index]]].start + P.F[P.famOf[s.S[j].index]].threshold < s.getMaxEnd(P))
    return 1;
  return 0;
}

int updateTime(const Problem& P, Solution& s, const int& i, const int& j, const int& firstOfLast,
	       const int& k, const int &m, std::vector<int>& endLast){
  int cur = 0; int nbJobs = j - firstOfLast + 1;
  std::vector<int> update(P.getFamilyNumber(), 0);
  s.S[firstOfLast].start = s.S[i].start + P.getDuration(s.S[i].index);
  s.S[firstOfLast].machine = m;
  for (cur = firstOfLast + 1; cur <= j; ++cur){
    s.S[cur].start = s.S[cur - 1].start + P.getDuration(s.S[cur - 1].index);
    s.S[cur].machine = m;
  }
  cur = 0;
  endLast[m] += nbJobs * P.getDuration(s.S[j].index);
  endLast[k] = 0;
  while (cur < P.N){
    // shift all the task after j by nbJobs * p_j on m
    if (s.S[cur].machine == m && s.S[cur].start >  s.S[j].start){
      s.S[cur].start += nbJobs * P.getDuration(s.S[j].index);
      if (s.QualifLostTime[P.famOf[s.S[cur].index]][m] < std::numeric_limits<int>::max() && !update[P.famOf[s.S[cur].index]]){
	update[P.famOf[s.S[cur].index]] = 1;
	s.QualifLostTime[P.famOf[s.S[cur].index]][m] += nbJobs * P.getDuration(s.S[j].index);
      }
    }
    //update endLast on k
    if (s.S[cur].machine == k && endLast[k] < s.S[cur].start + P.getDuration(s.S[cur].index))
      endLast[k] = s.S[cur].start + P.getDuration(s.S[cur].index);
    ++cur;

  }
  return 0;
}

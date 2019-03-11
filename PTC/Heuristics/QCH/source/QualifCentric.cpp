#include "QualifCentric.h"
#include <limits>
#include <algorithm>

using namespace QualifCentric;

int QCHWithOutput(Problem problem, Solution& s){
  Clock::time_point t1 = Clock::now();
  int solved = QCH(problem,s);
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

int QCH(Problem problem, Solution& s){
  std::vector<int> endLast(problem.M, 0);
  if (schedule(problem, s, endLast)){
    //std::cout << " Phase 1 done \n La solution est valide ?" << s.isValid(problem) << "\n" << s.toString(problem);
    //s.toTikz(problem);
    if (!intraChange(problem, s, endLast)){
      //std::cout << " Phase 2 done \n  La solution est valide ?" << s.isValid(problem) << "\n" << s.toString(problem); 
      //s.toTikz(problem);
      return !interChange(problem, s, endLast);
      return 1;
    }
    else return 0;
    return 1;
  } 
  else return 0;
}

namespace QualifCentric{
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

  int treat(Problem &problem, Solution& s, const int m, const int f, std::vector<int>& endLast, std::vector<int>& toSchedule, std::vector<int>& nextOfFam){
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

int schedule(Problem problem, Solution& s, std::vector<int>& endLast){
  const int F = problem.getNbFams();
  int i, f, j;
  std::vector<int> toSchedule(F, 0); //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);
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
      if ((f = chooseFamily(problem, s, j, endLast[j], toSchedule)) != -1){
	feasible = true;
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


  
int chooseFamily(const Problem &problem, const Solution& s, const int &m, const int& t,
		 std::vector<int> toSchedule){
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

int intraChange(const Problem &problem, Solution& s, std::vector<int>& endLast){
  int k, j, i = 0;
  const int F = problem.getNbFams();
  for (k = 0; k < problem.M; ++k){ //sur chaque machine
    if (s.getNbJobsOn(k) > 2){
      std::vector<int> moved(problem.N, 0);
      std::vector<int> firstOcc(F, -1);
      i = 0;
      //compute firstOcc of each family
      while (i < problem.N){
	if (s.S[i].machine == k &&
	    (firstOcc[problem.famOf[i]] == -1 ||
	     s.S[firstOcc[problem.famOf[i]]].start + problem.getDuration(firstOcc[problem.famOf[i]])
	     == s.S[i].start))
	  firstOcc[problem.famOf[s.S[i].index]] = i;
	++i;
      }
      //intraChange movement
      bool update;
      do {
	update = false;
	j = getLastOn(problem, s, k, endLast[k]); //recupere l'getIndex du dernier élément
	i = firstOcc[problem.famOf[s.S[j].index]];
	if (i != -1 && i != j && !moved[s.S[j].index] && !addDisqualif(problem, s, i, j, k, k, 1)){
	  update = true;
	  moved[s.S[j].index] = 1;
	  updateTime(problem, s, i, j, j, k, k, endLast);
	  firstOcc[problem.famOf[s.S[j].index]] = j;
	}
      } while (update);
    }
    int Cmax = s.getMaxEnd(problem);
    for (int f = 0; f < problem.getNbFams(); ++f)
      for (i = 0; i < problem.M; ++i){
	if (s.qualifLostTime[f][i] >= Cmax) s.qualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }
  
  return 0;
}

int interChange(const Problem& problem, Solution &s, std::vector<int>& endLast){
  int k, i, j;
  std::sort(s.S.begin(), s.S.end());
  for (k = 0; k < problem.M; ++k){// pour chaque machine
    if (s.getNbJobsOn(k) > 1){
      bool update;
      do {// tant qu'on fait un move
	update = false;
	// recuperer la derniere tache sur k (si il y en a une)
	if ((j = getLastOn(problem, s, k, endLast[k])) != problem.N){
	  int firstOfLast = getBeginOfLasts(problem, s, j);
	  int machineSelected = -1;
	  int jobSelected = -1;

	  findJobMachineMatch(problem,s,k,j,firstOfLast,endLast,machineSelected,jobSelected);
	
	  if (machineSelected != - 1 && jobSelected != -1) {
	    update = true;
	    updateTime(problem, s, jobSelected, j, firstOfLast, k, machineSelected, endLast);
	    std::sort(s.S.begin(), s.S.end());
	  }
	}
      } while (update);
    }
    
    int Cmax = s.getMaxEnd(problem);
    for (int f = 0; f < problem.getNbFams(); ++f)
      for (i = 0; i < problem.M; ++i){
	if (s.qualifLostTime[f][i] >= Cmax) s.qualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }
  return 0;
}

void findJobMachineMatch(const Problem& problem, const Solution& s, int k, int j, int firstOfLast, const std::vector<int>& endLast, int& machineSelected, int& jobSelected){
  for (int m = 0; m < problem.M; ++m){
    if (problem.isQualif(s.S[j].index, m) && k != m){// sur quelle machine je la met...
      int i = 0;
      while (i < problem.N){
	if (s.S[i].machine == m && problem.famOf[s.S[i].index] == problem.famOf[s.S[j].index])  //...et avec quelle tache
	  if (!addDisqualif(problem, s, i, firstOfLast, m, k, j - firstOfLast + 1)
	      && !addCompletion(problem, s.S[i].index, j - firstOfLast + 1, k, m, endLast)) 
	    if (machineSelected == -1 || endLast[m] < endLast[machineSelected]) {
	      jobSelected = i;
	      machineSelected = m ;
	    }
	++i;
      }
    }
  }
}

int addCompletion(const Problem& problem, const int& i, const int& nbJobs, const int& k, const int& m, std::vector<int> endLast){
  return (endLast[k] <= endLast[m] + nbJobs * problem.getDuration(i));
}

int getLastOn(const Problem& problem, const Solution& s, const int& k, const int& t){
  int i = 0;
  while (i < problem.N && (s.S[i].machine != k || s.S[i].start + problem.getDuration(s.S[i].index) != t))
    ++i;
  return i;
}

int getBeginOfLasts(const Problem& problem, const Solution& s, const int &last){
  int i = last;
  while (i >= 0 && problem.famOf[s.S[i].index] == problem.famOf[s.S[last].index])
    --i;
  return i + 1;
}


// i sur machine m ; j sur machine k
int addDisqualif(const Problem& problem, const Solution& s, const int& i, const int &j, const int& m,
		 const int & k, const int& nbJobs){
  const int F = problem.getNbFams();
  std::vector<int> firstOccAfter(F, problem.N);
  std::vector<int> lastOccBef(F, -1);
  
  //adding j on m add a disqualif? 

  //compute last occurence of each familiy before i on m
  computeLastOccBefore(problem,s,m,i,lastOccBef);
  //compute first occurence of each familiy after i on m
  computeFirstOccAfter(problem,s,m,i,firstOccAfter);
  
  //we just have two check if the "qualification" still holds between the last occ
  //of f before i and the first one after i
  for (int f = 0; f < F; ++f){
    if (f != problem.famOf[s.S[i].index] && problem.F[f].qualif[m]){
      if (lastOccBef[f] == -1){
	if (firstOccAfter[f] < problem.N)
	  if (s.S[firstOccAfter[f]].start + nbJobs * problem.getDuration(s.S[i].index) > problem.getThreshold(f))
	    return 1;
      }
      else
	if (firstOccAfter[f] < problem.N)
	  if (s.S[firstOccAfter[f]].start + nbJobs * problem.getDuration(s.S[i].index) -
	      s.S[lastOccBef[f]].start  > problem.getThreshold(f))
	    return 1;
    }
  }
  //removing j on k add a disqualif?
  lastOccBef[problem.famOf[s.S[j].index]]=-1;
  for (int cur = 0; cur < problem.N; ++cur){
    if (j != cur && problem.famOf[s.S[j].index] == problem.famOf[s.S[cur].index] && s.S[cur].machine == k && s.S[cur].start < s.S[j].start){
      if (lastOccBef[problem.famOf[s.S[cur].index]] == -1)
	lastOccBef[problem.famOf[s.S[cur].index]] = cur;
      else if (s.S[lastOccBef[problem.famOf[s.S[cur].index]]].start < s.S[cur].start)
	lastOccBef[problem.famOf[s.S[cur].index]] = cur;
    }
  }
  int Cmax = 0;
  if (k == m ) Cmax = s.getMaxEnd(problem);
  else{
    for (int l = 0 ; l < problem.M ; ++l){
      if ( l == k ) Cmax = std::max (Cmax,  s.getRealEnd(problem,k) - nbJobs * problem.getDuration(s.S[i].index));
      else if (l == m) Cmax = std::max (Cmax, s.getRealEnd(problem,m) + nbJobs* problem.getDuration(s.S[i].index));
      else Cmax = std::max(Cmax, s.getRealEnd(problem,l));
    }
  }
  if (lastOccBef[problem.famOf[s.S[j].index]] == -1){
    if (problem.F[problem.famOf[s.S[j].index]].threshold < Cmax )
      return 1;
  }
  else if (s.S[lastOccBef[problem.famOf[s.S[j].index]]].start + problem.F[problem.famOf[s.S[j].index]].threshold < Cmax)
    return 1;
  return 0;
}


void computeLastOccBefore(const Problem& problem, const Solution& s, int m, int i, std::vector<int>& lastOccBef){
  for (int cur = 0; cur < problem.N; ++cur)
    if (s.S[cur].machine == m && s.S[cur].start < s.S[i].start){
      if (lastOccBef[problem.famOf[s.S[cur].index]] == -1)
	lastOccBef[problem.famOf[s.S[cur].index]] = cur;
      else if (s.S[lastOccBef[problem.famOf[s.S[cur].index]]].start < s.S[cur].start)
	lastOccBef[problem.famOf[s.S[cur].index]] = cur;
    }
}

void computeFirstOccAfter(const Problem& problem, const Solution& s, int m, int i, std::vector<int>& firstOccAfter){
  for (int cur = 0; cur < problem.N; ++cur){
    if (s.S[cur].machine == m && s.S[cur].start > s.S[i].start){
      if (firstOccAfter[problem.famOf[s.S[cur].index]] == problem.N)
	firstOccAfter[problem.famOf[s.S[cur].index]] = cur;
      else if (s.S[firstOccAfter[problem.famOf[s.S[cur].index]]].start > s.S[cur].start)
	firstOccAfter[problem.famOf[s.S[cur].index]] = cur;
    }
  }
}

int updateTime(const Problem& problem, Solution& s, const int& i, const int& j, const int& firstOfLast,
	       const int& k, const int &m, std::vector<int>& endLast){
  int cur = 0;
  int nbJobs = j - firstOfLast + 1;
  std::vector<int> update(problem.getNbFams(), 0);
  s.S[firstOfLast].start = s.S[i].start + problem.getDuration(s.S[i].index);
  s.S[firstOfLast].machine = m;
  // ???
  for (cur = firstOfLast + 1; cur <= j; ++cur){
    s.S[cur].start = s.S[cur - 1].start + problem.getDuration(s.S[cur - 1].index);
    s.S[cur].machine = m;
  }

  cur = 0;
  endLast[m] += nbJobs * problem.getDuration(s.S[j].index);
  endLast[k] = 0;
  while (cur < problem.N){
    // shift all the task after j by nbJobs * p_j on m
    if (s.S[cur].machine == m && s.S[cur].start >=  s.S[firstOfLast].start){
      if (firstOfLast > cur || cur > j) 
	s.S[cur].start += nbJobs * problem.getDuration(s.S[j].index);
      if (s.qualifLostTime[problem.famOf[s.S[cur].index]][m] < std::numeric_limits<int>::max() && !update[problem.famOf[s.S[cur].index]]){
	update[problem.famOf[s.S[cur].index]] = 1;
	s.qualifLostTime[problem.famOf[s.S[cur].index]][m] += nbJobs * problem.getDuration(s.S[j].index);
      }
    }
    //update endLast on k
    if (s.S[cur].machine == k && endLast[k] < s.S[cur].start + problem.getDuration(s.S[cur].index))
      endLast[k] = s.S[cur].start + problem.getDuration(s.S[cur].index);
    ++cur;

  }
  return 0;
}

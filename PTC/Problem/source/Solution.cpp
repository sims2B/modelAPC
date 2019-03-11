
#include "Solution.h"

#include <limits>
#include <algorithm>

Solution::Solution(const Problem& P){
  const int nbFam = P.getNbFams();
  S.resize(P.N);
  qualifLostTime.resize(nbFam);
  for (int f = 0; f < nbFam; ++f)
    qualifLostTime[f].resize(P.M, std::numeric_limits<int>::max());
}

void Solution::clear(const Problem &P){
  const int nbFam = P.getNbFams();
  S.clear();
  qualifLostTime.clear();

  S.resize(P.N);
  qualifLostTime.resize(nbFam);
  for (int f = 0; f < nbFam; ++f)
    qualifLostTime[f].resize(P.M, std::numeric_limits<int>::max());
}
int Solution::getEnd(int j) const{
  int max = 0;
  for (unsigned int i = 0; i < S.size(); ++i)
    if (S[i].machine == j && S[i].start > max)
      max = S[i].start;
  return max;
}

int Solution::getRealEnd(const Problem& P, int j) const{
  int max = 0;
  for (unsigned int i = 0; i < S.size(); ++i)
    if (S[i].machine == j && S[i].start + P.getDuration(S[i].index) > max)
      max = S[i].start + P.getDuration(S[i].index);
  return max;
}

int Solution::getMaxEnd(const Problem& P) const{
  int max = 0;
  for (unsigned int i = 0; i < S.size(); ++i)
    if (S[i].start + P.getDuration(S[i].index)> max)
      max = S[i].start + P.getDuration(S[i].index);
  return max;
}

int Solution::getSumCompletion(const Problem &P) const{
  int sum = 0;
  for (unsigned int i = 0; i < S.size(); ++i)
    sum += S[i].start + P.getDuration(S[i].index);
  return sum;
}

std::vector<std::vector<int>> Solution::computeLastOf(const Problem & P) const{
  const int F = P.getNbFams();
  int j, f, i = 0;
  std::vector<std::vector<int>> lastOf(P.M);

  for (j = 0; j < P.M; ++j){
    lastOf[j].resize(F);
    for (f = 0; f < F; ++f)
      (P.F[f].qualif[j] ? lastOf[j][f] = 0 : lastOf[j][f] = std::numeric_limits<int>::max());
  }
  while (i < P.N){
    lastOf[S[i].machine][P.famOf[S[i].index]] = S[i].start;
    ++i;
  }
  return lastOf;
}

void Solution::repairDisqualif(const Problem& P){
  const int F = P.getNbFams();
  std::vector<std::vector<int>> lastOf = computeLastOf(P);

  int Cmax = getMaxEnd(P);
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < P.M; ++j){
      if (P.F[f].qualif[j]){
	if (lastOf[j][f] + P.getThreshold(f) < Cmax)
	  (lastOf[j][f] == 0 ? qualifLostTime[f][j] = P.getThreshold(f) : qualifLostTime[f][j] = lastOf[j][f] + P.getThreshold(f) );
	else qualifLostTime[f][j] = std::numeric_limits<int>::max();
      }
    }
}

int Solution::getNbDisqualif() const{
  int sum = 0;
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < std::numeric_limits<int>::max())
	sum++;
  return sum;
}

int Solution::getNbQualif(const Problem& P) const{
  int sum = 0;
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (P.F[f].qualif[j] && qualifLostTime[f][j] >= std::numeric_limits<int>::max())
	sum++;
  return sum;
}

int Solution::getRealNbDisqualif(const Problem& P) const{
  int sum = 0;
  int Cmax = getMaxEnd(P);
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < Cmax)
	sum++;
  return sum;
}

int Solution::getWeigthedObjectiveValue(const Problem &P) const{
  return alpha_C * getSumCompletion(P) + beta_Y * getNbDisqualif();
}

int Solution::getNbSetup(const Problem & P) const{
  Solution s2 = *(this);
  std::sort(s2.S.begin(), s2.S.end(),mchsComp);
  int nbSet = 0;

  for (int i = 0; i < P.N - 1; ++i)
    if (s2.S[i].machine == s2.S[i + 1].machine && P.famOf[s2.S[i].index] != P.famOf[s2.S[i + 1].index])
      nbSet++;
  return nbSet;
}

int Solution::getNbJobsOn(int m) const{
  int cpt = 0;
  for (unsigned int i = 0; i < S.size(); ++i){
    if (S[i].machine == m)
      cpt++;
  }
  return cpt;
}

int Solution::getJobs(int i, int m) const{
  unsigned int cur = 0;
  while (cur < S.size() && i > 0){
    if (S[cur].machine == m)
      i--;
    cur++;
  }
  return cur - 1;
}


//sort the solution in order to satisfy min_id = min_start (for CP warmStart)
int Solution::reaffectId(const Problem &P){
  std::sort(S.begin(), S.end(), startComp);
  std::vector<int> id;
  std::vector<int> sortedId;
  for (int f = 0; f < P.getNbFams(); ++f){
    for (int i = 0; i < P.N; ++i)
      if (P.famOf[S[i].index] == f) {
	id.push_back(i);
	sortedId.push_back(S[i].index);
      }
    std::sort(sortedId.begin(), sortedId.end());
    for (uint i = 0; i < sortedId.size(); ++i)
      S[id[i]].index = sortedId[i];
  }
  return 0;
}

//return 1 if interval [a,b] intersect [c,d]
int intersect(const int& a, const int& b, const int& c, const int& d){
  int t1 = std::max(c, a);
  int t2 = std::min(d, b);
  if (t1 >= t2) return 0;
  else return 1;
}

int Solution::isValid(const Problem &P) const{
  int i, j;
  const int n = P.N;
  const int F = P.getNbFams();
  std::vector<int> executed(P.N, 0);

  //std::cout << "all the tasks are executed\n";
  for (i = 0; i < P.N; ++i)
    executed[S[i].index] = 1;
  for (i = 0; i < P.N; ++i)
    if (!executed[i])
      return 0;

  //std::cout << "two tasks not executed in parallel on the same machine\n";
  for (i = 0; i < n; ++i)
    for (j = i + 1; j < n; ++j)
      if (S[i].machine == S[j].machine
	  && ((S[i].start <= S[j].start && S[i].start + P.getDuration(S[i].index) > S[j].start)
	      || (S[i].start <= S[j].start && S[i].start + P.getDuration(S[i].index) > S[j].start)))
	return 0;

  //std::cout << "setup time\n";
  for (i = 0; i < n; ++i)
    for (j = i + 1; j < n; ++j)
      if (S[i].machine == S[j].machine && P.famOf[S[i].index] != P.famOf[S[j].index])
	if (S[i].start + P.getDuration(S[i].index) + P.getSetup(S[j].index) > S[j].start
	    && S[j].start + P.getDuration(S[j].index) + P.getSetup(S[i].index) > S[i].start)
	  return 0;

  //std::cout << "when the task is processed, the machine is still qualified\n";
  for (i = 0; i < n; ++i)
    if (S[i].start + P.getDuration(S[i].index) > qualifLostTime[P.famOf[S[i].index]][S[i].machine] )
      return 0;

  //std::cout << "the machine j becomes disqualified for f if there is no task of f in an interval gamma_f\n";
  const int Cmax = getMaxEnd(P);
  for (j = 0; j < P.M; ++j)
    for (int f = 0; f < F; ++f)
      if (P.F[f].qualif[j])
	for (int t = P.getThreshold(f); t < Cmax; ++t){
	  i = 0;
	  while (i < n && !(P.famOf[S[i].index]) == f && S[i].machine == j && S[i].start > t - P.getThreshold(f) && S[i].start <= t)
	    ++i;
	  if (i >= n && qualifLostTime[f][j] > t){
	    //std::cout << "probleme sur " << f << " " << j << " au tps " << t << std::endl;
	    return 0;
	  }
	}

  //std::cout << " no more than gamma_f between to task of the same family\n";
  for (i = 0; i < n; ++i){
    j = 0;
    bool vu = false;
    while (j < n && (i == j || P.famOf[S[i].index] != P.famOf[S[j].index]  || S[i].machine != S[j].machine
		     || !(S[j].start > S[i].start && S[j].start <= S[i].start + P.getThreshold(S[i].index)))){
      if (i != j && P.famOf[S[j].index] == P.famOf[S[i].index] && S[i].machine == S[j].machine && S[j].start > S[i].start) vu = true;
      ++j;
    }
    if (vu)
      if (j >= n)
	return 0;
  }
  return 1;
}


std::string Solution::toString(const Problem& P) const{
  std::string res = "Une solution est : \n - les dates de début des tâches : \n";
  for (unsigned int i = 0; i < S.size(); ++i)
    res += " * la tache " + std::to_string(S[i].index) + " commence au temps " + std::to_string(S[i].start)
      /*+ " et finit au temps "
	+ std::std::to_string(S[i].start + P.getDuration(i))*/
      + " sur la machine " + std::to_string(S[i].machine) + "\n";
  res += " - Les familles ayant perdues leur qualification sont :\n";
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f){
    res += " * la famille " + std::to_string(f) + " a perdu sa qualif sur les machines:\n ";
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < std::numeric_limits<int>::max())
	res += "\t - " + std::to_string(j) + " au temps " + std::to_string(qualifLostTime[f][j]) + "\n";
    res += "\n";
  }
  res += "Le nombre de Machines disqualifiées est : " + std::to_string(getNbDisqualif()) + " et la sum des completion times vaut :" + std::to_string(getSumCompletion(P)) + "\n";
  return res;
}

void Solution::toTikz(const Problem& P) const{
  std::cout << "\\begin{tikzpicture}\n" <<
    "\\node (O) at (0,0) {};\n" <<
    "\\draw[->] (O.center) -- ( " << getMaxEnd(P) + 1 << ",0);\n" <<
    "\\draw[->] (O.center) -- (0, " << (P.getNbFams() * 0.5 + 0.5) << ");\n";
  for (int t = 5; t <= getMaxEnd(P); t = t + 5)
    std::cout << "\\draw (" << t << ",0) -- (" << t << ",-0.1) node[below] {$" << t << "$} ;\n";
  for (unsigned int i = 0; i < S.size(); ++i){
    std::cout << "\\draw[fill = " << tikzColor[P.famOf[S[i].index]] <<
      "!80!black!80]  (" << S[i].start << "," << (S[i].machine * 0.5) <<
      ") rectangle (" << S[i].start + P.getDuration(S[i].index) << ","
	      << (S[i].machine * 0.5 + 0.5) << ") node[midway] {$" <<
      S[i].index << "$};\n";
  }
  std::cout << "\\end{tikzpicture}\n";
}

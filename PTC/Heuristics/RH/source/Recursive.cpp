#include "Recursive.h"
#include <cmath>
#include <limits>
 

std::vector<bool> convert(int x,int size);
// !!! ajouté les disqualifications à la solution, i.e. QualifLostTime?? !!!
int RH(const Problem& P, Solution& s_res, ptrHeur Heuristic){
  Solution s(P);
  if (Heuristic(P,s)){
    s_res = s;
    int nbDisqualif = 0;
    std::vector<std::tuple<int,int>> disqualif(P.M*P.getFamilyNumber());
    for (uint f = 0 ; f < s.QualifLostTime.size() ; ++f)
      for (uint j = 0 ; j < s.QualifLostTime[f].size() ; ++j)
	if (s.QualifLostTime[f][j] < std::numeric_limits<int>::max()){
	  std::tuple<int,int> lost(f,j);
	  disqualif[nbDisqualif]=lost;
	  nbDisqualif++;
	}
    disqualif.resize(nbDisqualif);
   
    if (nbDisqualif){
      int stop = pow(2,nbDisqualif);
      int iter = 1;
      while (iter < stop){
	Solution s_temp(P);
	Problem Q(P);
        if (!disqualify(Q,disqualif,iter,nbDisqualif))
	  if (Heuristic(Q,s_temp)){
	    // MAJ qualifLostTime w.r.t. P (modif in Q)
	    for (uint f = 0 ; f < s.QualifLostTime.size() ; ++f)
	      for (uint j = 0 ; j < s.QualifLostTime[f].size() ; ++j)
		if (P.F[f].qualif[j] != Q.F[f].qualif[j])
		  s_temp.QualifLostTime[f][j] = P.F[f].threshold;
	    if (s_temp.getWeigthedObjectiveValue(Q,alpha,beta)
		< s_res.getWeigthedObjectiveValue(P,alpha,beta))
	      s_res = s_temp;
	  }
	iter ++;
      }
    }
    return 1;
  }
  else return 0;
}

std::vector<bool> convert(int x,int size) {
  std::vector<bool> ret;
  while(x) {
    if (x&1)
      ret.push_back(1);
    else
      ret.push_back(0);
    x>>=1;  
  }
  ret.resize(size);
  reverse(ret.begin(),ret.end());
  return ret;
}

int disqualify(Problem& Q, const std::vector<std::tuple<int,int>>& disqualif,
	       const int& iter, const int& nbDisqualif){
  std::vector<bool> toDisqualif = convert(iter,nbDisqualif);
  for (uint i = 0 ; i < toDisqualif.size() ; ++i)
    if (toDisqualif[i])
      Q.F[std::get<0>(disqualif[i])].qualif[std::get<1>(disqualif[i])] = 0;
  return 0;
}

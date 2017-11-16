#include "Solution.h"
#include <iostream>
#include <limits>
#include <algorithm>

Solution::Solution(const Problem& P){
  const int nbFam = P.getFamilyNumber();
  S.resize(P.N);
  QualifLostTime.resize(nbFam);
  for (int f = 0 ; f < nbFam ; ++f)
    QualifLostTime[f].resize(P.M,std::numeric_limits<int>::max());
}


int Solution::getEnd(int j) const{
  int max = 0;
  for (uint i = 0 ; i < S.size() ; ++i)
   if (S[i].machine == j && S[i].start > max)
     max = S[i].start;
  return max;
}



int Solution::getSumCompletion(const Problem &P) const{
  int sum = 0;
  for (uint i = 0 ; i < S.size() ; ++i)
    sum += S[i].start + P.getDuration(i);
  return sum;
}

int Solution::getNbDisqualif() const{
  int sum = 0;
  for (uint f = 0 ; f < QualifLostTime.size() ; ++f)
    for (uint j = 0 ; j < QualifLostTime[f].size() ; ++j)
      if (QualifLostTime[f][j] < std::numeric_limits<int>::max())
	sum++;
  return sum;
}
int Solution::getRealNbDisqualif(const Problem& P) const{
  int sum = 0;
  for (uint f = 0 ; f < QualifLostTime.size() ; ++f)
    for (uint j = 0 ; j < QualifLostTime[f].size() ; ++j)
      if (QualifLostTime[f][j] < getEnd(j) + P.F[f].duration)
	sum++;
  return sum;
}

int Solution::getWeigthedObjectiveValue(const Problem &P, const int & alpha, const int& beta) const{
  return alpha * getSumCompletion(P) + beta * getNbDisqualif();
}

int Solution::getNbSetup(const Problem & P) const{
  Solution s2 = *(this);
  std::sort(s2.S.begin() , s2.S.end());
  int nbSet = 0;

  for (int i = 0; i < P.N - 1 ; ++i)
    if (s2.S[i].machine == s2.S[i+1].machine && P.famOf[s2.S[i].index] !=  P.famOf[s2.S[i+1].index])
      nbSet++;
  return nbSet;
}

//return 1 if interval [a,b] intersect [c,d]
int intersect( const int& a, const int& b,  const int& c, const int& d){
  int t1 = std::max(c, a);
  int t2 = std::min(d ,b);
  if ( t1 >= t2 ) return 0;
  else return 1;
}

int Solution::isValid(const Problem &P) const{
  int i,j;
  const int n = P.N;
  const int F = P.getFamilyNumber();
  
   //std::cout<<"two tasks not executed in parallel on the same machine\n";
  for (i = 0 ; i < n ; ++i)
    for (j = i + 1 ; j < n ; ++j)
      if (S[i].machine == S[j].machine 
	  && ((S[i].start <= S[j].start && S[i].start + P.getDuration(i) > S[j].start)
	      || (S[i].start <= S[j].start && S[i].start + P.getDuration(i) > S[j].start)))
	return 0;
  
   //std::cout<<"setup time\n";
  for (i = 0 ; i < n ; ++i)
    for (j = i + 1 ; j < n ; ++j)
      if (S[i].machine == S[j].machine && P.famOf[i] != P.famOf[j])
	if (S[i].start + P.getDuration(i) + P.getSetup(j) > S[j].start 
	    && S[j].start + P.getDuration(j) + P.getSetup(i) > S[i].start)
	  return 0;

   //std::cout<<"when the task is processed, the machine is still qualified\n";
  for (i = 0 ; i < n ; ++i)
    if (S[i].start + P.getDuration(i) > QualifLostTime[P.famOf[i]][S[i].machine])
      return 0;

  //std::cout<<"the machine j becomes disqualified for f if there is no task of f in an interval gamma_f\n";
  for (j = 0 ; j < P.M ; ++j)
    for (int f = 0 ; f < F ; ++f)
      if (P.F[f].qualif[j])
	for (int t = 0 ; t < getEnd(j) - P.F[f].threshold ; ++t){
	  i = 0;
	  while (!(P.famOf[i]==f && intersect(S[i].start,S[i].start+P.getDuration(i), t ,t + P.F[f].threshold+1))
		 && i < n)
	    ++i;
	  if (i >= n && QualifLostTime[f][j] > t + P.F[f].threshold){
	    return 0;
	  }
	}
  
  //std::cout<<" no more than gamma_f between to task of the same family\n";
  for (i = 0 ; i < n ; ++i){
    j = 0;
    bool vu = false;
    while (j < n && (i==j || P.famOf[i]!= P.famOf[j] || S[i].machine != S[j].machine
		     || !intersect(S[i].start + P.getDuration(i) ,
				   S[i].start + P.getDuration(i)+ P.getThreshold(i)+1,
				   S[j].start , S[j].start + P.getDuration(i) )
		     )){
      if (i != j && P.famOf[j] == P.famOf[i] &&  S[i].machine == S[j].machine && S[j].start > S[i].start) vu = true; 
      ++j;
    }
    if (vu)
      if (j >= n)
	return 0;
  }
  return 1;
}


std::string Solution::toString() const{
  std::string res= "Une solution est : \n - les dates de début des tâches : \n" ;
  for (uint i =0 ; i < S.size() ; ++i)
    res+= " * la tache " + std::to_string(i) + " commence au temps " + std::to_string(S[i].start)
      /*+ " et finit au temps "
	+ std::to_string(S[i].start + P.getDuration(i))*/
      + " sur la machine " + std::to_string(S[i].machine) + "\n";
  res+= " - Les familles ayant perdues leur qualification sont :\n";
  for (uint f = 0 ; f < QualifLostTime.size() ; ++f){
    res+= " * la famille " + std::to_string(f) + " a perdu sa qualif sur les machines:\n " ;
    for (uint j = 0 ; j < QualifLostTime[f].size() ; ++j)
      if (QualifLostTime[f][j] < std::numeric_limits<int>::max())
	res+= "\t - " + std::to_string(j) + " au temps " + std::to_string(QualifLostTime[f][j]) + "\n";
    res+= "\n";
  }
  return res;
}

#include "problem.h"
#include <random>
#include <cassert>
#include <algorithm>

Family::Family(int m) : duration(0), setup(0), threshold(0) {
  qualif.clear();
  qualif.resize(m,0);
}

Family::Family(int _dur,int _setup, int _threshold,  int m):
  duration(_dur), setup(_setup), threshold(_threshold){
  qualif.resize(m,0);
}

Family::Family(int _dur,int _setup, int _threshold, std::vector<int> _qualif):
  duration(_dur), setup(_setup), threshold(_threshold), qualif(_qualif){}

int Family::writeFamily(std::ofstream& out) const{
  out <<  duration << " " << threshold << " " << setup << " " << qualif[0];
  for (unsigned int i = 1 ; i < qualif.size() ; ++i)
    out << " " << qualif[i]; 
  out << std::endl;
  return 0;
}

std::string Family::toString() const{
  std::string res = "La famille a une durée de " + std::to_string(duration) + ", un setup time de " + std::to_string(setup) + 
	  ", un threshold de " + std::to_string(threshold) + " et les machines ";
  for (unsigned int i = 0 ; i < qualif.size() ; ++i){
    if (qualif[i])
      res+= std::to_string(i) + " " ;
  }
  res += " sont qualifiées pour son execution\n";
  return res;
}

Family readFamily(std::ifstream& in, int M){
  Family f(M);
  in >> f.duration >> f.threshold >> f.setup ;
  for (int i = 0 ; i < M ; ++i){
    int q;
    in >> q;
    f.qualif[i] = q;
  }
  return f;
}


Problem::Problem(int nbTask, int nbFam): N(nbTask), M(0){
  F.reserve(nbFam);
  famOf.resize(nbTask);
}

Problem::Problem(int nbTask, int nbMach, int nbFam): N(nbTask), M(nbMach){
  Family _F(M);
  F.resize(nbFam,_F);
  famOf.resize(nbTask);
}

Problem::Problem(int nbTask, int nbMach, std::vector<int> _famOf,
		 familyList _F): N(nbTask), M(nbMach), famOf(_famOf), F(_F){}

int Problem::writeInFile(std::ofstream& out) const{
  unsigned int i ;
  out << N << " " << M << " " << getFamilyNumber()
      << std::endl;
  out << famOf[0];
  for (i = 1 ; i < famOf.size() ; ++i)
    out << " " << famOf[i];
  out << std::endl;
  for (i = 0 ; i < F.size(); ++i)
    F[i].writeFamily(out);
  return 0;
}

int Problem::computeHorizon() const{
  int res=0;
  for (int i = 0 ; i < N ; ++i)
    res+= getDuration(i) + getSetup(i);
  return res;
}

std::string Problem::toString() const{
  std::string res = "Le problème possède les caractéristiques suivantes:\n - le nombre de tâches est :" + std::to_string(N) + 
	  "\n - le nombre de machine est :" + std::to_string(M) + "\n - \n";
  for (int i = 0 ; i < N ; ++i)
    res += "  * la tâche " + std::to_string(i) + " appartient à la famille " + std::to_string(famOf[i]) + "\n";
  res+= "- Et les familles (" + std::to_string(F.size()) + ") possèdent les caractéristiques suivantes:\n";
  for (unsigned int f = 0 ; f < F.size() ; ++f)
    res+= "   * " + std::to_string(f) + ": " + F[f].toString();
  return res;
}

int Problem::getNf(int f) const{
  int res = 0;
  for (int i = 0 ; i < N ; ++i)
    if (famOf[i]==f)
      res++;
  return res;
}

void Problem::toDimacs() const {
  std::cout << "c MACHINES "<< M << std::endl
  << "c FAMILIES "<< getFamilyNumber() << std::endl
  << "c JOBS "<<N << std::endl;
}

//reader
Problem readFromFile(std::ifstream& in){
  unsigned int i , F , N, M;
  in >> N >> M >> F;
  Problem P(N,M,F);
  for (i = 0 ; i < N ; ++i)
    in >> P.famOf[i] ;
  for (i = 0 ; i < F ; ++i)
    P.F[i] = readFamily(in,M);
  std::sort(P.famOf.begin(),P.famOf.end());
  return P;
}

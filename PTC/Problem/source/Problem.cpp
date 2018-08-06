
#include "Problem.h"
#include <random>
#include <cassert>
#include <algorithm>

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

//Ali
void generateDuration(Problem& P, const int& F, const int& pmax){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disDur(1,pmax);
  // std::normal_distribution<> disDur(4.0*(double)pmax/5.0,(double)pmax/5.0);
  int sample;  
  for (int f = 0 ; f < F ; ++f){
    // do {
      sample = (int)disDur(generator);
      //} while (sample < 1 || sample > pmax);
    P.F[f].duration = sample;
  }
}

void generateSetup(Problem& P, const int& F, const int& pmin){
  std::random_device rd;
  std::mt19937 generator(rd());
  int sample,f;

  std::uniform_int_distribution<int> disSet(1,pmin);
  //std::normal_distribution<> disSet(pmin/2 ,pmin/5);
  for (f = 0 ; f < F ; ++f){
    // do {
      sample = (int)disSet(generator);
      // } while (sample < 1 || sample > pmin);
    P.F[f].setup = sample;
  }
}


void generateThreshold(Problem& P/*, const int& n*/, const int& m, const int& F, const int & pmax, const int& smax){
  int Hbound = P.computeHorizon() / m ;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disThresh(std::max((cat-1)*Hbound/nbCat ,
							sizeMin * 2 * pmax + smax),
					       std::max(cat *Hbound/nbCat,
							(sizeMin+1) * 2 * pmax + 2*smax));
  for (int f = 0 ; f < F ; ++f){
    int sample = disThresh(generator) ;
    //std :: cout << sample << std::endl;
    if (sample < P.F[f].duration + smax + sizeMin * pmax + P.F[f].setup)
      P.F[f].threshold = P.F[f].duration + smax + sizeMin * pmax + P.F[f].setup;
    if (sample > P.F[f].duration + smax + (sizeMin+1) * pmax + P.F[f].setup)
      P.F[f].threshold =  P.F[f].duration  + smax + (sizeMin + 1) * pmax + P.F[f].setup;
    else P.F[f].threshold = sample;
  }
 }


void generateQualif(Problem& P, const int& m, const int& F, int sumQualif){
  std::random_device rd;
  std::mt19937 generator(rd());
  int sample,j,f= 0;
  std::vector<int> selected(F,0);
  std::vector<int> nbQualif(F,0);
  std::uniform_int_distribution<int> qualifPerFam(0, F - 1);
  std::uniform_int_distribution<int> machQualif(0,m - 1);

  //nbQualifPerFamily generation
  for (f = 0 ; f < F ; ++f)
    nbQualif[f]++;
  sumQualif -= F;
  
  while (sumQualif > 0){
    sample = (int) qualifPerFam(generator);
    while (nbQualif[sample] == m){
      sample ++ ;
      sample = sample % F;
    }
    nbQualif[sample]++;
    sumQualif--;
  }

  //each machine choose a family
  for (j =0 ; j < m ; ++j){
    sample =  qualifPerFam(generator);
    while (nbQualif[sample] == 0){
      sample ++ ;
      sample = sample % F;
    }
    nbQualif[sample]--;
    selected[sample]++;
    P.F[sample].qualif[j] = 1;
  }

  //std::cout << "nbQualif per families generated\n";
  //the rest of the affectation is done randomly
  for (f = 0 ; f < F ; ++f){
    j = m - selected[f];
    while (nbQualif[f] > 0){
      int index =  (int)(machQualif(generator) % j);
      int select = -1;
      while (index > -1 ){
	if (!P.F[f].qualif[select+1])
	  index--;
	select++;
	select=select% m;
      } 
      P.F[f].qualif[select] = 1;
      j--; nbQualif[f]--;
    }
  }
}


void generateFamilies(Problem& P,const int& m,
		      const int& F, const int& pmax, int sumQualif){
  generateDuration(P,F,pmax);
  int minDur = std::numeric_limits<int>::max();
  for (int f = 0 ; f < F ; ++f)
    if (P.F[f].duration < minDur) minDur = P.F[f].duration;
  generateSetup(P,F,minDur);
  
  int maxDur = std::numeric_limits<int>::min();
  int maxSet = std::numeric_limits<int>::min();
  for (int f = 0 ; f < F ; ++f){
    if (P.F[f].setup > maxSet) maxSet = P.F[f].setup;
    if (P.F[f].duration > maxDur) maxDur = P.F[f].duration;
  }
  generateThreshold(P,m,F,maxDur,maxSet);
  
  generateQualif(P,m,F,sumQualif);
}

Problem generateProblem(const int& n, const int& m, const int& F,
			const int& pmax, int sumQualif){
  assert( sumQualif  >= m && sumQualif >= F && sumQualif <= F*m); 
  Problem P(n , m , F);
  generateFamilies(P,m,F,pmax,sumQualif);
  affectFamily(P,n,F);
  return P;
}
//common

void affectFamily(Problem& P, const int& n, const int& F){
  int nbRes = n;
  std::random_device rd;
  std::mt19937 generator(rd());
  int sample, f;
  std::uniform_int_distribution<int> disNf(0,F-1);
  std::vector<int> nF(F,0); // nbTask per family
  for (f = 0 ; f < F ; ++f)
    nF[f]++;
  nbRes -= F;
  while (nbRes > 0){
    sample = disNf(generator);
    nF[sample]++;
    nbRes--;
  }

  int sum = 0;
  for (f = 0 ; f < F ; ++f){
    int i = 0;
    while (i < nF[f]){
      P.famOf[sum + i] = f;
      i++;
    }
    sum += nF[f];
  }
  
  /*
  std::uniform_int_distribution<int> famAffect(0,n-1);
  std::vector<int> selected(n,0);
  nbRes = n;
  for (f = 0 ; f < F ; ++f){
    while (nF[f] > 0){
      int index = (int)(famAffect(generator) % nbRes);
      int select = -1;
      while (index > -1){
	if (!selected[select+1])
	  index--;
	select++;
      }
      selected[select] = 1;
      P.famOf[select] = f;
      nF[f]--; nbRes--;
    }
    }*/
}

//Abdoul
void generateFamilies(Problem& P/*, const int& n*/, const int& m,
		      const int& F){
  generateDuration(P,F);
  generateSetup(P,F);
  generateThreshold(P,m,F);
  generateQualif(P,m,F);
}


void generateSetup(Problem& P, const int& F){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disSetup(Smin,Smax);
  for (int  f = 0 ; f < F ; ++f){
    P.F[f].setup = disSetup(generator);
  }  
}

void generateThreshold(Problem& P,  const int& m, const int& F){
  int Hbound = P.computeHorizon() / m ;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disThresh(std::max((cat-1)*Hbound/nbCat ,
							sizeMin * Pmax + Smax),
					       std::max(cat*Hbound/nbCat,
							(sizeMin+1) * Pmax + 2*Smax));
  for (int f = 0 ; f < F ; ++f){
    int sample = disThresh(generator) ;
    if (sample < sizeMin * Pmax + Smax + P.F[f].setup)
      P.F[f].threshold = sizeMin * Pmax + Smax + P.F[f].setup;
    if (sample > (sizeMin+1) * Pmax + Smax + P.F[f].setup)
      P.F[f].threshold = (sizeMin+1) * Pmax + Smax + P.F[f].setup;
    else P.F[f].threshold = sample;
  }
}
  
void generateDuration(Problem& P,const int& F){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disDur(Pmin,Pmax);
  for (int  f = 0 ; f < F ; ++f){
    P.F[f].duration = disDur(generator);
  }
}

void generateQualif(Problem& P, const int& m, const int& F){

  std::random_device rd;
  std::mt19937 generator(rd());
  int sample,j,f = 0;
  std::uniform_int_distribution<int> qualifPerFam(1,m);
  std::vector<int> nbQualif(F,0);

  //nbQualifPerFamily generation
  for (f = 0 ; f < F ; ++f){
    sample = qualifPerFam(generator);
    nbQualif[f] = sample;
  }
  
  //machine affectation to family
  std::uniform_int_distribution<int> machQualif(0,m-1);
  std::vector<int> selected(m,0);
  for (f = 0 ; f < F ; ++f){
    j = m;
    while (nbQualif[f] > 0){
      int index =  (int)(machQualif(generator) % j);
      int select = -1;
      while (index > -1 ){
	if (!P.F[f].qualif[select+1])
	  index--;
	select++;
      } 
      P.F[f].qualif[select] = 1;
      selected[select] = 1;
      j--; nbQualif[f]--;
    }
  }

  //if a machine cannot execute any family, randomly affect a family
  std::uniform_int_distribution<int> disFam(0,F-1);
  for (j = 0 ; j < m ; ++j){
    if ( selected[j] == 0 ){
      int fam = disFam(generator);
      P.F[fam].qualif[j]=1;
    }
  }
}
Problem generateProblem(const int& n, const int& m, const int& F){
  Problem P(n,m,F);
  generateFamilies(P,m,F);
  affectFamily(P,n,F);
  return P;
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

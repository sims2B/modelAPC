#include "Problem.h"
#include <random>
#include <limits>
#include <cassert>
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
  uint i ;
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
  std::string res = "Le problème possède les caractéristiques suivantes:\n - le nombre de tâches est :" + std::to_string(N) + "\n - le nombre de machine est :" + std::to_string(M) + "\n - \n";
  for (int i = 0 ; i < N ; ++i)
    res += "  * la tâche " + std::to_string(i) + " appartient à la famille " + std::to_string(famOf[i]) + "\n";
  res+= "- Et les familles (" + std::to_string(F.size()) + ") possèdent les caractéristiques suivantes:\n";
  for (uint f = 0 ; f < F.size() ; ++f)
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

void generateFamilies(Problem& P, const int& n, const int& m,
		      const int& F, const int& pmax, int sumQualif){
  generateDuration(P,F,pmax);

  int minDur = std::numeric_limits<int>::max();
  for (int f = 0 ; f < F ; ++f)
    if (P.F[f].duration < minDur) minDur = P.F[f].duration;
  generateSetup(P,F,minDur);

  generateThreshold(P,n,m,F);
  generateQualif(P,m,F,sumQualif);
}

void generateDuration(Problem& P, const int& F, const int& pmax){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::normal_distribution<> disDur(4.0*(double)pmax/5.0,(double)pmax/5.0);
  int sample;  
  for (int f = 0 ; f < F ; ++f){
    do {
      sample = (int)disDur(generator);
    } while (sample < 1 || sample > pmax);
    P.F[f].duration = (int)sample;
  }
}

void generateSetup(Problem& P, const int& F, const int& pmin){
  std::random_device rd;
  std::mt19937 generator(rd());
  int sample,f;

  std::normal_distribution<> disSet(pmin/2 ,pmin/5);
  for (f = 0 ; f < F ; ++f){
    do {
      sample = (int)disSet(generator);
    } while (sample < 1 || sample > pmin);
    P.F[f].setup = (int)sample;
  }
}

void generateThreshold(Problem& P, const int& n, const int& m, const int& F){
  std::random_device rd;
  std::mt19937 generator(rd());
  int sample,f;
  int maxDur = std::numeric_limits<int>::min();
  for ( f = 0 ; f < F ; ++f)
    if (P.F[f].duration  + P.F[f].setup > maxDur)
      maxDur = P.F[f].duration + P.F[f].setup;

  std::normal_distribution<> disThres(3*(n+m)*maxDur/(8*m) ,3*(n-m)*maxDur/(24*m) );
  for ( f = 0 ; f < F ; ++f){
    do {
      sample = (int)disThres(generator);
    } while (sample < maxDur || sample > 3*n*maxDur/(4*m));
    P.F[f].threshold = (int)sample;
  }
}

void generateQualif(Problem& P, const int& m, const int& F, int sumQualif){

  std::random_device rd;
  std::mt19937 generator(rd());
  int sample, j, f ,nbFull = 0;
  std::uniform_int_distribution<int> qualifPerFam(0,F-1);
  std::vector<int> nbQualif(F,0);
  for (f = 0 ; f < F ; ++f)
    nbQualif[f]++;
  sumQualif -= F;
  while (sumQualif > 0){
    sample = (int)(qualifPerFam(generator) % (m - nbFull));
    int select = -1 ;
    while (sample > -1 ){
      if (nbQualif[select+1] < m)
	sample--;
      select++;
    } 
    nbQualif[select]++;
    if (nbQualif[select]==m) nbFull++;
    sumQualif--;
  }
  //machine affectation to family
  std::uniform_int_distribution<int> machQualif(0,m-1);
  std::vector<int> selected(m,0);
  j=m; f=0;
  //make sure each machine is qualified for at least one family
  while (j > 0 && f < F){
    do {
      int index =  (int)(machQualif(generator)%j) ;
      int select = -1;
      while (index > -1 ){
	if (!selected[select+1])
	  index--;
	select++;
      } 
      selected[select] = 1;
      P.F[f].qualif[select] = 1;
      j--; nbQualif[f]--;
    } while (nbQualif[f] > 0 && j > 0);
    if (j > 0) f++;  
    else {
      while (nbQualif[f] > 0){
	int select = (int)(machQualif(generator) % m);
	if (P.F[f].qualif[select] != 1){
	  P.F[f].qualif[select] = 1;
	  nbQualif[f]--;
	} 
      } 
    }
  }
  //rest of the family
  for (int rest = f + 1 ; rest < F ; ++rest){
    j=m;
    while (nbQualif[rest] > 0){
      int index =  (int)(machQualif(generator) % j);
      int select = -1;
      while (index > -1 ){
	if (!P.F[rest].qualif[select+1])
	  index--;
	select++;
      } 
      P.F[rest].qualif[select] = 1;
      j--; nbQualif[rest]--;
    }
  }
}

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
  }
}

Problem generateProblem(const int& n, const int& m, const int& F,
			const int& pmax, int sumQualif){
  assert( sumQualif  >= m && sumQualif >= F); 
  Problem P(n,m,F);
  generateFamilies(P,n,m,F,pmax,sumQualif);
  affectFamily(P,n,F);
  return P;
}


//Abdoul
void generateFamilies(Problem& P, const int& n, const int& m,
		      const int& F){
  int Hbound = 0 , i = 0;
  generateDuration(P,F);
  generateSetup(P,F);
  for (i = 0 ; i < n ; ++i)
    Hbound += P.getDuration(i) /*+ P.getSetup(i)*/;
  Hbound = Hbound / m;
  generateThreshold(P,F,Hbound);
  generateQualif(P,m,F);
}

void generateThreshold(Problem& P, const int& F,const int& Hbound){
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

void generateSetup(Problem& P, const int& F){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> disSetup(Smin,Smax);
  for (int  f = 0 ; f < F ; ++f){
    P.F[f].setup = disSetup(generator);
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
  generateFamilies(P,n,m,F);
  affectFamily(P,n,F);
  return P;
}


//reader
Problem readFromFile(std::ifstream& in){
  uint i , F , N, M;
  in >> N >> M >> F;
  Problem P(N,M,F);
  for (i = 0 ; i < N ; ++i)
    in >> P.famOf[i] ;
  for (i = 0 ; i < F ; ++i)
    P.F[i] = readFamily(in,M);
  return P;
}

#ifndef PROBLEM_H
#define PROBLEM_H

#include "Family.h"
#include <iostream>
#include <fstream>

//parametre pour la generation d'instance!!
//min/max duration
const int Pmax = 10;
const int Pmin = 1;
//min/max setup
const int Smax = 5;
const int Smin = 1;
//threshold 
const int nbCat = 3; // small, meduim and big
const int cat = 3; //1 = small, 2 = meduim , 3 = big
const int sizeMin = 3; // "minimum threshold for at least sizeMin task"

typedef std::vector<Family> familyList;

//WARNING!!!! Family in F are "ordered", don't change their order

struct Problem{
  //a problem is then defined by its number of taks, its number of machines and 
  //a list of families (containing duration, setup, threshold, number of task
  // and qualified machines)
  int N;
  int M;
  std::vector<int> famOf;
  familyList F;

  //////////CONSTRUCTOR///////
  //Construct a problem with a predefined number of task, number of machine equals to 0
  //set the capacity of the vector F to the number of families and of famOf to the
  //number of task
  Problem(int N,int F);
  //idem above plus number of machine
  Problem(int N,int M,int F);
  //Construct a problem with the value of the parameter given in argument
  Problem(int N,int M,std::vector<int> f, familyList F);

  int writeInFile(std::ofstream&) const;
  //get the number of task in a family
  int getNf(int f) const;
  
  //display solution
  std::string toString() const;
  
  //get an upper bound on the time horizon (to improve)
  int computeHorizon() const;
  
  //get the number of family
  inline int getFamilyNumber() const{
    return (int)this->F.size();
  }
  //get a task duration
  inline int getDuration(int i) const{
    return this->F[famOf[i]].duration;
  }
  //get a task setup time 
  inline int getSetup(int i) const{
    return this->F[famOf[i]].setup;
  }
  //get a task threshold
  inline int getThreshold(int i) const{
    return this->F[famOf[i]].threshold;
  }
  //return 1 if machine m is qualified for task i
  inline int isQualif(int i, int m) const{
    return this->F[this->famOf[i]].qualif[m];
  }

};

///////////////////////////////////////////////////
/////////////////// generateAli ///////////////////
///////////////////////////////////////////////////
Problem generateProblem(const int& n, const int& m, const int& F, 
						const int& pmax, int sumQualif);
void generateFamilies(Problem& P, const int& m, const int& F, const int& pmax, 
					int sumQualif);
//distribution uniforme [1,pmax]
void generateDuration(Problem& P,const int& F,const int& pmax);
//distribution uniforme [1,pmin]
void generateSetup(Problem& P, const int& F, const int& pmin);
void generateQualif(Problem& P, const int& m, const int& F, int sumQualif);
void generateThreshold(Problem& P,const int& m, const int& F,const int& pmax, 
						const int& smax);
///////////////////////////////////////////////////
///////////////////// common //////////////////////
///////////////////////////////////////////////////
void affectFamily(Problem& P, const int& n, const int& F);

///////////////////////////////////////////////////
////////////////// generateAbdoul /////////////////
///////////////////////////////////////////////////

Problem generateProblem(const int& n, const int& m, const int& F);
void generateFamilies(Problem& P,const int& m,const int& F);
void generateThreshold(Problem& P, const int& F, const int& Hbound);
void generateThreshold(Problem& P,const int& m, const int& F);
void generateDuration(Problem& P,const int& F);
void generateSetup(Problem& P, const int& F);
void generateQualif(Problem& P, const int& m, const int& F);


//reader
Problem readFromFile(std::ifstream&);
#endif


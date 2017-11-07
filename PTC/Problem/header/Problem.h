#ifndef PROBLEM_H
#define PROBLEM_H

#include "Family.h"
#include <iostream>
#include <fstream>
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
    return this->F.size();
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


//generate new instance
void generateFamilies(Problem& P, const int& n, const int& m,
		      const int& F, const int& pmax, int sumQualif);
void generateThreshold(Problem& P, const int& n, const int& m, const int& F);
void generateDuration(Problem& P,const int& F,const int& pmax);
void generateSetup(Problem& P, const int& F);
void generateQualif(Problem& P, const int& m, const int& F, int sumQualif);
void affectFamily(Problem& P, const int& n, const int& F);

Problem generateProblem(const int& n, const int& m, const int& F,
			const int& pmax, int sumQualif);

Problem readFromFile(std::ifstream&);
#endif


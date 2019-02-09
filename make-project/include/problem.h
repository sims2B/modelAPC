#ifndef PROBLEM_H
#define PROBLEM_H

#include <iostream>
#include <fstream>
#include <vector>

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
const int sizeMin = cat; // "minimum threshold for at least sizeMin task" (small => 1 ; medium => 2 ; big => 3)


class Family{

  //a family is described by the duration of a task belonging to this family,
  //the setup time needed before executing a task of this family (independent
  //of the task scheduled before), the threshold (after which a machine loose
  //its qualification) and the set of machine on which a task of this family
  //can be executed
 public:
  // TODO Use private field ?  
  int duration;
  int setup;
  int threshold;
  std::vector<int> qualif;

  //////////CONSTRUCTOR///////
  //Construct a family with all parameter = to 0 but set the size of the vector
  //to the number of machine
  Family(int M);
  //Construct a family with the value of the parameter given in argument and set the
  //size of the vector to the number of machine
  Family(int dur ,int set ,int thre ,int M);
  //Construct a family with the value of the parameter given in argument
  Family(int dur,int set,int thre,std::vector<int> Q);

  int writeFamily(std::ofstream&) const;
  std::string toString() const;
};

typedef std::vector<Family> familyList;

Family readFamily(std::ifstream&, int M);


//WARNING!!!! Family in F are "ordered", don't change their order
// FIXME in this, why is it even possible to change their order !!!

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

//Problem File Reader
Problem readFromFile(std::ifstream&);

#endif

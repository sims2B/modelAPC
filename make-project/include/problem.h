#ifndef PROBLEM_H
#define PROBLEM_H

#include <fstream>
#include <iostream>
#include <vector>

const int infinity = std::numeric_limits<int>::max();

class Job {
 private:
  const int fam;
  int start;
  int index;

 public:
  Job(int fam);
  Job(int, int, int);
  Job &operator=(const Job &j);

  inline int getFam() const { return fam; }
  inline int getStart() const { return start; }
  inline int getIndex() const { return index; }

  inline void shiftStart(int s){start += s;};

  std::string toString() const;
};

bool operator==(const Job &, const Job &);
bool operator<(const Job &, const Job &);

class Family {
 private:
  int duration;
  int setup;
  int threshold;
  int nbJobs;
  std::vector<int> qualif;

 public:
  //////////CONSTRUCTOR///////
  Family(int M);
  // Construct a family with the value of the parameter given in argument and
  // set the size of the vector to the number of machine
  Family(int dur, int set, int thre, int nf, int M);
  // Construct a family with the value of the parameter given in argument
  Family(int dur, int set, int thre, int nf, std::vector<int> Q);

  // get
  inline int getDuration() const { return duration; }
  inline int getSetup() const { return setup; }
  inline int getThreshold() const { return threshold; }
  inline int getNbJobs() const { return nbJobs; }
  inline std::vector<int> getQualif() const { return qualif; }
  inline int isQualif(int m) const { return qualif[m]; }
  inline void disqualif(int m) { qualif[m] = 0; }
  inline void qualify(int m) { qualif[m] = 1; }
  inline void decreaseNbJob() { nbJobs--; }
  inline void setDuration(int dur) { duration = dur; }
  inline void setSetup(int set) { setup = set; }
  inline void setThreshold(int gamma) { threshold = gamma; }
  inline void setNbJob(int nb) { nbJobs = nb; }
  int writeFamily(std::ofstream &) const;
  std::string toString() const;
};

Family readFamily(std::ifstream &, int M);
Family oldReader(std::ifstream &, int M, int size);

typedef std::vector<Family> familyList;
// WARNING!!!! Family in F are "ordered", don't change their order
// FIXME in this, why is it even possible to change their order !!!
class Problem {
  // a problem is then defined by its number of taks, its number of machines and
  // a list of families (containing duration, setup, threshold, number of task
  // and qualified machines)
 private:
  int N;
  int M;
  familyList F;

  //////////CONSTRUCTOR///////
  // Construct a problem with a predefined number of task, number of machine
  // equals to 0 set the capacity of the vector F to the number of families and
  // of famOf to the number of task
 public:
  Problem(int N, int F);
  // idem above plus number of machine
  Problem(int N, int M, int F);
  // Construct a problem with the value of the parameter given in argument
  Problem(int N, int M, familyList F);
  Problem(const Problem &);

  int writeInFile(std::ofstream &) const;
  // get the number of task in a family
  inline int getNf(int f) const { return F[f].getNbJobs(); }
  int getFamily(int i) const;
  // display solution
  std::string toString() const;

  // get an upper bound on the time horizon (to improve)
  int computeHorizon() const;

  inline int getNbJobs() const { return N; }
  inline int getNbMchs() const { return M; }
  inline int getNbFams() const { return (int)F.size(); }
  inline familyList getFamList() const { return F; }

  // family getter
  inline int getDuration(int f) const { return F[f].getDuration(); }
  inline int getSetup(int f) const { return F[f].getSetup(); }
  inline int getThreshold(int f) const { return F[f].getThreshold(); }
  inline int isQualif(int f, int m) const { return F[f].isQualif(m); }
  inline int getNbJobs(int f) const { return F[f].getNbJobs(); }
  inline int getDuration(Job i) const { return F[i.getFam()].getDuration(); }

  int getSetup(Job i) const { return F[i.getFam()].getSetup(); }
  int getThreshold(Job i) const { return F[i.getFam()].getThreshold(); }
  int getFam(Job i) const { return i.getFam(); }

  inline void disqualif(int f, int m) { F[f].disqualif(m); }
  inline void qualify(int f, int m) { F[f].qualify(m); }
  inline void decreaseNbJobs(int f) { F[f].decreaseNbJob(); }

  inline void setDuration(int f, int duration) { F[f].setDuration(duration); }
  inline void setSetup(int f, int set) { F[f].setSetup(set); }
  inline void setThreshold(int f, int gamma) { F[f].setThreshold(gamma); }
  inline void setNbJobs(int nb, int f){ F[f].setNbJob(nb);}
 void toDimacs() const;
};

Problem readFromFile(std::ifstream &);
Problem oldReader(std::ifstream &);

// Problem readProblemFile(std::string pathname);

#endif

#ifndef SOLUTION_H
#define SOLUTION_H

#include "paramModelAPC.h"
#include "problem.h"

// Color Palette used for TiKZ drawings
#define TIKZ_COLORS 15
const std::string tikzColors[TIKZ_COLORS] = {"cyan", "lime", "olive", "magenta", "teal", "orange", "lightgray", "pink", "purple", "red", "blue", "violet", "yellow", "brown", "green"};

typedef std::vector<std::vector<Job>> JobMatrix;
typedef std::vector<std::vector<int>> IntMatrix;

class Solution {
  
 private:
  Problem problem;
  JobMatrix assign;
  IntMatrix qualifLostTime;

 public:
  Solution(const Problem&);  // construct right size vector (resize).
                             // qualifLostTime initialized with +oo.
  void clear();

  void addJob(const Job& j, int m);
  void removeLastJob(int m);
  void removeJob(const Job& i, int m);
  Job getLastJob(int j) const; 
  int getLastStart(int j) const;  // last getStart time on machine j
  int getEnd(int j) const;
  int getMaxEnd() const;  // compute CMax
  int getSumCompletion() const;
  void repairDisqualif();

  Job lastOf(int f, int m) const;  // return last job of f scheduled on m (if
  // not return job(f,-1,-1))
  Job firstOf(int f, int m) const;  // return last job of f scheduled on m (if
  // not return job(f,-1,-1)
  void getFirstOcc(Job& j, int f, int& m)
      const;  // pour le model CP : etant donnée une famille, renvoie le premier
              // job de la famille et la machine sur la quelle le job est
              // schedule
  Job getPreviousOcc(const Job&, int f, int m) const;

  Job nextOf(int i, int f, int m) const; //return first Job of f scheduled after job at ith pos
  Job nextOf(const Job& i, int f, int m) const; //return first Job of f scheduled after j

  //int getTotalNbDisqualif() const;
  int getNbQualif() const;
  int getNbDisqualif() const;
  int getWeigthedObjectiveValue() const;
  int getNbSetup(int m) const;
  int getNbSetup() const;
  int getNbJobsOn(int m) const;

  Job getJobs(int i, int m) const;  // return the ith jobs on m (start at 0)
  void shiftJob(int i , int m , int shift);
  inline void setDisqualif(int val, int f, int m) {
    qualifLostTime[f][m] = val;
  }
  inline int getDisqualif(int f, int m) const { return qualifLostTime[f][m]; }

  int isValid() const;  // return 1 if the solution satisfies all the
                        // constraints of the problem

  std::string toString() const;
  void toTikz() const;
  void toDimacs() const;
};

bool compareLexFQ(const Solution&, const Solution&);

bool compareLexQF(const Solution&, const Solution&);

// bool compareWSum(const Solution&, const Solution&); // TODO will need a
// struct or class for the coefficients

#endif

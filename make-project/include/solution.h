#ifndef SOLUTION_H
#define SOLUTION_H

#include "paramModelAPC.h"
#include "problem.h"

// TODO Try to avoid the restriction on the max number of families !
// REP : c'est just limitant pour le tikz! Je me sert uniquement de la borne
// pour définir les couleurs.
#define nbFamMax 5
const std::string tikzColor[nbFamMax] = {"blue", "green", "orange", "purple",
                                         "red"};
typedef std::vector<std::vector<Job>> JobMatrix;
typedef std::vector<std::vector<int>> IntMatrix;

class Solution {
  // FIXME Change visibility of some fields and methods
  // FIXME the problem should be stored in a field ?

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
  inline Job getLastJob(int j) const { return assign[j][assign[j].size() - 1]; }
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

  Job nextOf(int i, int f, int m) const;
  Job nextOf(const Job& i, int f, int m) const;

  int getTotalNbDisqualif() const;
  int getNbQualif() const;
  int getNbDisqualif() const;
  int getWeigthedObjectiveValue() const;
  int getNbSetup(int m) const;
  int getNbSetup() const;
  int getNbJobsOn(int m) const;

  Job getJobs(int i, int m) const;  // return the ith jobs on m (if sorted)

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

// bool compareLexFQ(const Solution&, const Solution&);

// bool compareLexQF(const Solution&, const Solution&);

// bool compareWSum(const Solution&, const Solution&); // TODO will need a
// struct or class for the coefficients

#endif

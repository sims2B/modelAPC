
#ifndef SOLUTION_H
#define SOLUTION_H
#include "Problem.h"
#include "Assignment.h"

#define nbFamMax 5
const std::string tikzColor[nbFamMax] = { "blue", "green", "orange", "purple","red"};
typedef std::vector<Assignment> assignmentList;

struct Solution{
  //a problem is then defined by a List of Assignment (containing for each task
  //its start time and affectation to a machine) and a matrix
  //family x machines containing the time where a machine become disqualified for a
  //family f (0 if the machine is not qualified from the beginning) if it exists
  //and +oo otherwise
  assignmentList S;
  std::vector<std::vector<int>> QualifLostTime;

  //construct right size vector (resize). QualifLostTime initialized with +oo.
   Solution(const Problem&);
   void clear(const Problem &P);
  //last start time on machine j
   int getEnd(int j) const; 
   int getRealEnd(const Problem& P, int j) const;
  //compute CMax
  int getMaxEnd(const Problem& P) const;
  int getSumCompletion(const Problem&) const;
  void repairDisqualif(const Problem&);
  //compute the matrix of last job of family f on machine m (start time)
  //the solution must be sorted by 1/machine; 2/start
  std::vector<std::vector<int>> computeLastOf(const Problem & P) const;

  int getNbDisqualif() const;
  int getRealNbDisqualif(const Problem& P) const;
  int getWeigthedObjectiveValue(const Problem&, const int&, const int&) const;
  int getNbSetup(const Problem&) const;
  int getNbJobsOn(int m) const; 
  // return the ith jobs on m (if sorted)
  int getJobs(int i, int m) const; 
  int reaffectId(const Problem &P);
  //return 1 if the solution satisfies all the constraints of the problem
  int isValid(const Problem&) const;

  std::string toString(const Problem& P) const;
  void toTikz(const Problem&) const;

};
#endif

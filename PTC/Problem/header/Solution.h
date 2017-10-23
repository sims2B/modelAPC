
#ifndef SOLUTION_H
#define SOLUTION_H
#include "Problem.h"
#include "Assignment.h"

typedef std::vector<Assignment> assignmentList;

struct Solution{
  //a problem is then defined by a List of Assignment (containing for each task
  //its start time and affectation to a machine) and a matrix
  //family x machines containing the time where a machine become disqualified for a
  //family f (0 if the machine is not qualified from the beginning), if it exists
  //and +oo otherwise
  assignmentList S;
  std::vector<std::vector<int>> QualifLostTime;

  //construct right size vector (resize). QualifLostTime initialized with +oo.
   Solution(const Problem&);

  int getEnd(int j) const;
  int getSumCompletion(const Problem&) const;
  int getNbDisqualif() const;
  int getWeigthedObjectiveValue(const Problem&, const int&, const int&) const;
  //return 1 if the solution satisfies all the constraints of the problem
  int isValid(const Problem&) const;

  std::string toString() const;
};
#endif

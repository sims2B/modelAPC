#ifndef FAMILY_H
#define FAMILY_H

#include <fstream>

struct Family{

  //a family is described by the duration of a task belonging to this family,
  //the setup time needed before executing a task of this family (independent
  //of the task scheduled before), the threshold (after which a machine loose
  //its qualification) and the set of machine on which a task of this family
  //can be executed
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

Family readFamily(std::ifstream&, int M);
#endif

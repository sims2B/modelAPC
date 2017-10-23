#ifndef FAMILY_H
#define FAMILY_H

#include <string>
#include <vector>

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
  Family(int);
  //Construct a family with the value of the parameter given in argument and set the
  //size of the vector to the number of machine
  Family(int,int,int,int);
  //Construct a family with the value of the parameter given in argument
  Family(int,int,int,std::vector<int>);

  std::string toString() const;
};
  
#endif

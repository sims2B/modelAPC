#ifndef ASSIGNMENT_H
#define ASSIGNEMENT_H

struct Assignment{
  int start;
  int machine;
  int index;

  Assignment();
  Assignment(int,int,int);
};
bool operator==(const Assignment&, const Assignment&);
bool operator<(const Assignment&, const Assignment&);
#endif

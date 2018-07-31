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
bool operator<(const Assignment& a1, const Assignment& a2);
bool mchsComp(const Assignment&, const Assignment&);

bool startComp(const Assignment&, const Assignment&);
bool idComp(const Assignment&, const Assignment&);
#endif

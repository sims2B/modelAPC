#include "Assignment.h"

Assignment::Assignment() : start(-1), machine(-1), index(-1){}


Assignment::Assignment(int _start, int _mach, int _index) : start(_start), machine(_mach), index(_index){}

bool operator==(const Assignment& a1, const Assignment& a2){
  return (a1.start == a2.start && a1.machine == a2.machine);  
}

bool operator<(const Assignment& a1, const Assignment& a2){
  return (a1.machine < a2.machine || (a1.machine == a2.machine && a1.start < a2.start));
}

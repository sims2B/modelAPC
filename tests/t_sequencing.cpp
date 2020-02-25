#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

bool testSequenceSMPT(SequenceSMPT &seq, int flowtime) {
  seq.sequencing();
  seq.printSequence();
  std::cout << std::endl;
 
  return seq.searching() == flowtime ;
  // && seq.searching(flowtime + flowtime /10)
  // && seq.searching(flowtime) 
  // && !seq.searching(flowtime - 1);
}

bool testSequence0() {
  int n = 3;
  int durations[] = {2, 3, 4};
  int setups[] = {10, 5, 1};
  SequenceSMPT seq(n, durations, setups);
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 0);
   }
  if(! testSequenceSMPT(seq, 0)) return false;
  
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 1);
   }
  if(! testSequenceSMPT(seq, 24)) return false;

  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 10);
   }
  if(! testSequenceSMPT(seq, 1305)) return false;
  
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 0);
   }
  if(! testSequenceSMPT(seq, 0)) return false;
  
  return true;
}


bool testSequence1() {
  int n = 5;
  int durations[] = {4, 2, 3, 2, 4};
  int setups[] = {1, 2, 3, 1, 5};
  int required[] = {3, 2, 1, 0, 5};
  SequenceSMPT seq(n, durations, setups);
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, required[i-1]);
   }
  return testSequenceSMPT(seq, 100);
}

int main()
{
   if(!testSequence0()) {
    return(EXIT_FAILURE);   
   }

  //  if(!testSequence1()) {
  //   return(EXIT_FAILURE);   
  //  }
 std::cout << std::endl << "TEST SUCCESS" << std::endl; 
  return(EXIT_SUCCESS);
}

#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"


void setRequired(SequenceSMPT &seq, int n, int required) {
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, required);
   }
}

void setRequired(SequenceSMPT &seq, int n,int required[]) {
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, required[i-1]);
   }
}

bool testSequenceSMPT(SequenceSMPT &seq, int flowtime) {
  seq.sequencing();
  seq.printSequence();
  const int results = seq.searching();
  std::cout << "COMPUTED FLOWTIME "<< results << std::endl;
  
  return results == flowtime 
   && seq.searching(flowtime + flowtime /10)
   && seq.searching(flowtime) 
   && (flowtime <= 0 || !seq.searching(flowtime - 1));
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
  SequenceSMPT seq(n, durations, setups);

  int required[] = {3, 2, 1, 0, 5};
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, required[i-1]);
   }
  if( !testSequenceSMPT(seq, 260)) return false;

  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 1);
  }
  if(! testSequenceSMPT(seq, 58)) return false;

  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 0);
  }
  if(! testSequenceSMPT(seq, 0)) return false;

  return true;
}

bool testExtendedSequence0() {
  int n = 3;
  int durations[] = {2, 3, 4};
  int setups[] = {10, 5, 1};
  SequenceSMPT seq(n, durations, setups, true);
  for (int i = 1; i <= 2*n; i++) {
     seq.setRequired(i, 0);
   }
  if(! testSequenceSMPT(seq, 0)) return false;
  
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, 1);
   }
  if(! testSequenceSMPT(seq, 24)) return false;

  for (int i = 1; i <= 2*n; i++) {
     seq.setRequired(i, 1);
   }
  if(! testSequenceSMPT(seq, 64)) return false;
 
  return true;
}

bool testSequence2() {
  int n = 4;
  int durations[] = {1, 2, 3, 4};
  int setups[] = {1, 1, 1, 1};
  SequenceSMPT seq(n, durations, setups);
  
  setRequired(seq, n, 1);
  seq.setRequired(1, 10);
  if(! testSequenceSMPT(seq, 107)) return false;
 
  setRequired(seq, n, 1);
  seq.setRequired(2, 10);
  if(! testSequenceSMPT(seq, 188)) return false;
 
  setRequired(seq, n, 1);
  seq.setRequired(3, 10);
  if(! testSequenceSMPT(seq, 260)) return false;

  setRequired(seq, n, 1);
  seq.setRequired(4, 10);
  if(! testSequenceSMPT(seq, 323)) return false;

  return true;
}

int main()
{
  //if(!testSequence2() ) {    
  if(!testSequence0() || !testSequence1() || !testSequence2() || !testExtendedSequence0()) {
    std::cout << std::endl << "TEST FAILURE" << std::endl; 
    return(EXIT_FAILURE);   
   }
  std::cout << std::endl << "TEST SUCCESS" << std::endl; 
  return(EXIT_SUCCESS);
}

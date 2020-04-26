#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

int main() {
  int n = 3;  
  int durations[] = {2, 3, 4};
  int setups[] = {5, 3, 1};
  SequenceSMPT seq(n, durations, setups, true);
  seq.setRequired(1, 1);
  seq.setRequired(2, 1);
  seq.setRequired(3, 1);
  seq.searching();
  seq.toTikz();

  seq.setRequired(2, 3);
  seq.searching();
  //seq.printSequence();
  seq.toTikz();

  seq.setRequired(2, 1);
  seq.setRequired(3 + 1, 2);
  seq.setRequired(3 + 2, 1);
  seq.searching();
  //seq.printSequence();
  seq.toTikz();


  return (EXIT_SUCCESS);
}

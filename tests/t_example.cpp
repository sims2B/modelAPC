#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

// Define in SequenceSMPT.h
// #define DEBUG_SEQ
void search(SequenceSMPT& seq) {
  SequenceData res = seq.search();
  seq.toTikz(0);
  seq.toTikz(res.firstFamily);
}

int main() {
  int n = 3;  
  int durations[] = {2, 3, 4};
  int setups[] = {5, 3, 1};
  SequenceSMPT seq(n, durations, setups, true);
  seq.setRequired(1, 1);
  seq.setRequired(2, 1);
  seq.setRequired(3, 1);
  search(seq);

  seq.setRequired(2, 3);
  search(seq);

  seq.setRequired(2, 1);
  seq.setRequired(3 + 1, 2);
  seq.setRequired(3 + 2, 1);
  search(seq);


  return (EXIT_SUCCESS);
}

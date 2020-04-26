#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

void searching(SequenceSMPT& seq) {
  std::cout << "Flowtime Without Initial Setup : " << seq.searching() << std::endl << std::endl;
  seq.toTikz(0);
  seq.toTikz(1);
    seq.toTikz(2);
      seq.toTikz(3);
}

int main() {
  int n = 3;  
  int durations[] = {2, 3, 4};
  int setups[] = {5, 3, 1};
  SequenceSMPT seq(n, durations, setups, true);
  seq.setRequired(1, 1);
  seq.setRequired(2, 1);
  seq.setRequired(3, 1);
  searching(seq);

  seq.setRequired(2, 3);
  searching(seq);

  seq.setRequired(2, 1);
  seq.setRequired(3 + 1, 2);
  seq.setRequired(3 + 2, 1);
  searching(seq);


  return (EXIT_SUCCESS);
}

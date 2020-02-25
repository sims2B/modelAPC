#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

int main()
{
  int n = 5;
  int durations[] = {4, 2, 3, 2, 4};
  int setups[] = {1, 2, 3, 1, 5};
  int required[] = {3, 2, 1, 0, 5};

  SequenceSMPT seq(n, durations, setups);
  seq.printSequence();
  std::cout << std::endl;
  for (int i = 1; i <= n; i++) {
     seq.setRequired(i, required[i-1]);
   }
   seq.printSequence();
   std::cout << std::endl;
 
   seq.sequencing();
   seq.printSequence();
 std::cout << std::endl;
   
  return(EXIT_SUCCESS);
}

#include "sequenceSMPT.h"

#include <algorithm>

#define DEBUG
// #define FILTER_FAMILY

bool compareWeights(FamilyRun* f1, FamilyRun* f2) { return f1->getWeight() < f2->getWeight(); }

void SequenceSMPT::sequencing() {
 std::sort(sequence.begin()++, sequence.end(), compareWeights);

  for(int i = 1; i <= n ; i++) {
    sequence[i]->scheduleAfter(sequence[i-1]);
  }
  #ifdef DEBUG
  printSequence();
  #endif

}
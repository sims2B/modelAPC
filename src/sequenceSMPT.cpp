#include "sequenceSMPT.h"

#include <algorithm>

#define DEBUG_SEQ

bool compareWeights(FamilyRun* f1, FamilyRun* f2) {
  return f1->getWeight() < f2->getWeight();
}

int SequenceSMPT::sequencing() {
  // Init.
  int flowtimeWS = 0;
  sequence[0]->setNext(size);
  // Sort according to SMPT order
  std::sort(sequence.begin()++, sequence.end(), compareWeights);

  // Schedule the runs
  for (int i = 1; i <= n; i++) {
    sequence[i]->scheduleAfter(sequence[i - 1]);
    flowtimeWS += sequence[i]->getFlowtime();
  }
  #ifdef DEBUG_SEQ 
  std::cout << "FLOWTIME_WS " << flowtimeWS << std::endl;
  printSequence();
  #endif
  return flowtimeWS;
}

int SequenceSMPT::searchNextRun(int from) {
  while (++from <= n) {
    if (sequence[from]->required > 0) return from;
  }
  return from;
}

int SequenceSMPT::searching() {
  if (size == 0) return 0;
  const int flowtimeWS = sequencing();
  // Remove setup from the first non empty run
  int i = searchNextRun(0);
  int bestDelta = sequence[i]->cancelSetup();
  #ifdef DEBUG_SEQ 
   printf("cancel setup F%d -> %d\n", sequence[i]->index, bestDelta);
  #endif
  
  // Try to schedule first each remaining non empty run
  while ((i = searchNextRun(i)) <= n) {
    const int delta = sequence[i]->moveFirst();
  #ifdef DEBUG_SEQ 
   printf("schedule first F%d -> %d\n", sequence[i]->index, delta);
  #endif
    if (delta < bestDelta) {
      bestDelta = delta;
    }
  }
  const int flowtime = flowtimeWS + bestDelta;
  #ifdef DEBUG_SEQ 
  std::cout << "FLOWTIME " << flowtime << std::endl;
  #endif
  return flowtime;
}

bool SequenceSMPT::searching(int flowtimeUB) {
  if (size == 0) return true;
  const int flowtimeWS = sequencing();
  // The sequence with initial setup is feasible 
  if (flowtimeWS <= flowtimeUB) return true;
  // the flowtime must decrease so that the sequence becomes feasible
  const int targetDelta = flowtimeUB - flowtimeWS;
  #ifdef DEBUG_SEQ 
   printf("FLOWTIME_DELTA %d\n", targetDelta);
  #endif
  // Remove setup from the first non empty run
  int i = searchNextRun(0);
  int delta = sequence[i]->cancelSetup();
  
  if (delta <= targetDelta) return true;

  // Try to schedule first each non empty run
  while ((i = searchNextRun(i)) <= n) {
    if (sequence[i]->moveFirst() <= targetDelta) return true;
  }
  // No feasible schedule found
  #ifdef DEBUG_SEQ 
   printf("FAIL\n");
  #endif
  return false;
}

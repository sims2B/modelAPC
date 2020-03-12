#include "sequenceSMPT.h"

#include <algorithm>

#define DEBUG

bool compareWeights(FamilyRun* f1, FamilyRun* f2) {
  return f1->getWeight() < f2->getWeight();
}

void SequenceSMPT::sequencing() {
  // Init.
  flowtimeWS = 0;
  sequence[0]->initFirstRun(nreq);

  // Sort according to SMPT order
  std::sort(sequence.begin()++, sequence.end(), compareWeights);

  // Schedule the runs
  for (int i = 1; i <= n; i++) {
    sequence[i]->scheduleAfter(sequence[i - 1]);
    flowtimeWS += sequence[i]->getFlowtime();
  }
}

int SequenceSMPT::searchNextRun(int from) {
  while (++from <= n) {
    if (sequence[from]->required > 0) return from;
  }
  return from;
}

int SequenceSMPT::searching() {
  if (nreq == 0) return 0;
  // Remove setup from the first non empty run
  int i = searchNextRun(0);
  int bestDelta = sequence[i]->cancelSetup();
  // printf("try F%d -> %d\n", sequence[i]->index, bestDelta);

  // Try to schedule first each remaining non empty run
  while ((i = searchNextRun(i)) <= n) {
    const int delta = sequence[i]->moveFirst();
    // printf("try F%d -> %d\n", sequence[i]->index, delta);
    if (delta < bestDelta) {
      bestDelta = delta;
    }
  }
  return flowtimeWS + bestDelta;
}

bool SequenceSMPT::searching(int flowtimeUB) {
  if (nreq == 0 || flowtimeWS <= flowtimeUB) return true;
  const int targetDelta = flowtimeUB - flowtimeWS;
  // printf("try target delta -> %d\n", targetDelta);

  // Remove setup from the first non empty run
  int i = searchNextRun(0);
  int delta = sequence[i]->cancelSetup();
  if (delta <= targetDelta) return true;

  // Try to schedule first each non empty run
  while ((i = searchNextRun(i)) <= n) {
    if (sequence[i]->moveFirst() <= targetDelta) return true;
  }
  // No feasible schedule found
  // printf("fail target delta -> %d\n", targetDelta);
  return false;
}

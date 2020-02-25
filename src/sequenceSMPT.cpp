#include "sequenceSMPT.h"

#include <algorithm>

#define DEBUG
// #define FILTER_FAMILY

bool compareWeights(FamilyRun* f1, FamilyRun* f2) { return f1->getWeight() < f2->getWeight(); }

void SequenceSMPT::sequencing() {
  // Init. 
  flowtimeWS = 0;
  sequence[0]->initFirstRun(nreq);

 // Sort according to SMPT order 
 std::sort(sequence.begin()++, sequence.end(), compareWeights);

  // Schedule the runs
  for(int i = 1; i <= n ; i++) {
    sequence[i]->scheduleAfter(sequence[i-1]);
    printf("FLOWTIME %d\n", sequence[i]->getFlowtime());
    flowtimeWS += sequence[i]->getFlowtime();
  }
  printf("FLOWTIME_WS %d\n", flowtimeWS);
}
  

int SequenceSMPT::searchNextRun(int from) {
      while( ++from <= n) {
      if(sequence[from]->required > 0) return from;
      } 
      return from;
}
 
 int SequenceSMPT::searching() {
   // start from the first non empty run
   if(nreq == 0) return 0;
   int i = searchNextRun(0);
   // Subtract the initial setup to the flowtime
   int bestDelta =  sequence[i]->cancelSetup();
   printf("try F%d -> %d\n", sequence[i]->index, bestDelta);
        
   // Try to schedule first each non empty run
   while( (i = searchNextRun(i) ) <= n) {
         const int delta = sequence[i]->moveFirst();
         printf("try F%d -> %d\n", sequence[i]->index, delta);
         if(delta < bestDelta) {bestDelta = delta;}
    } 
    return flowtimeWS + bestDelta;
   }


bool SequenceSMPT::searching(int flowtimeUB) {
  return false;
}
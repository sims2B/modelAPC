#include "sequenceSMPT.h"

#include <algorithm>

//#define DEBUG_SEQ

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
  std::cout << "FLOWTIME_WITH_INITIAL_SETUP " << flowtimeWS << std::endl;
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


// Question : if a family has no setup, then it is useless to try to move it in first position.
// TODO Find a way to return the family in first position
SequenceData SequenceSMPT::search() {
  SequenceData res ={0, 0, 0};
  if (size == 0) return res;
  res.flowtimeWS = sequencing();
  // Remove setup from the first non empty run
  int i = searchNextRun(0);
  res.firstFamily = sequence[i]->index;
  res.flowtime = sequence[i]->cancelSetup();
  #ifdef DEBUG_SEQ 
   printf("cancel setup F%d -> %d\n", sequence[i]->index, res.flowtime);
  #endif
  
  // Try to schedule first each remaining non empty run
  while ((i = searchNextRun(i)) <= n) {
    const int delta = sequence[i]->moveFirst();
  #ifdef DEBUG_SEQ 
   printf("schedule first F%d -> %d\n", sequence[i]->index, delta);
  #endif
    if (delta < res.flowtime) {
       res.firstFamily = sequence[i]->index;
       res.flowtime = delta;
    }
  }
  res.flowtime += res.flowtimeWS;
  #ifdef DEBUG_SEQ 
  std::cout << "FLOWTIME " << res.flowtime << " WITH_FIRST_FAMILY " << res.firstFamily << std::endl;
  #endif
  return res;
}

bool SequenceSMPT::search(int flowtimeUB) {
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


void SequenceSMPT::toTikz(int first) {
const int makespan = sequence.back()->endtime + 2;
 std::cout << "\\begin{tikzpicture}\n"
            << "\\node (O) at (0,0) {};\n"
            << "\\draw[->] (O.center) -- ( " << makespan << ",0);\n"
            << "\\draw[->] (O.center) -- (0, 2);\n";
    
 if(first > 0) {    
      int start = runs[first]->toTikz(0, false);
      for (auto run : sequence) {
        if(run->index != first) start = run->toTikz(start, true);
      }
 } else {
      int start = 0;
      for (auto run : sequence) {
        start = run->toTikz(start, true);
      }
    }
       std::cout << "\\end{tikzpicture}\n\n";
    }
    


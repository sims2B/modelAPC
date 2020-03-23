#include <cstdlib>
#include <iostream>
#include "sequenceSMPT.h"

void setRequired(SequenceSMPT &seq, int n, int required) {
  for (int i = 1; i <= n; i++) {
    seq.setRequired(i, required);
  }
}

void setRequired(SequenceSMPT &seq, int n, int required[]) {
  for (int i = 1; i <= n; i++) {
    seq.setRequired(i, required[i - 1]);
  }
}

bool testSequenceSMPT(SequenceSMPT &seq, int flowtime) {
  seq.sequencing();
  seq.printSequence();
  const int results = seq.searching();
  std::cout << "COMPUTED FLOWTIME " << results << std::endl << std::endl;

  return results == flowtime && seq.searching(flowtime) &&
         (flowtime <= 0 || !seq.searching(flowtime - 1));
}

bool testSequence0() {
  int n = 3;
  int durations[] = {2, 3, 4};
  int setups[] = {10, 5, 1};
  SequenceSMPT seq(n, durations, setups);
  setRequired(seq, n, 0);
  if (!testSequenceSMPT(seq, 0))
    return false;

  setRequired(seq, n, 1);
  if (!testSequenceSMPT(seq, 24))
    return false;

  setRequired(seq, n, 10);
  if (!testSequenceSMPT(seq, 1305))
    return false;

  setRequired(seq, n, 0);
  if (!testSequenceSMPT(seq, 0))
    return false;

  return true;
}

bool testSequence1() {
  int n = 5;
  int durations[] = {4, 2, 3, 2, 4};
  int setups[] = {1, 2, 3, 1, 5};
  SequenceSMPT seq(n, durations, setups);

  int required[] = {3, 2, 1, 0, 5};
  setRequired(seq, n, required);

  if (!testSequenceSMPT(seq, 260))
    return false;

  setRequired(seq, n, 1);
  if (!testSequenceSMPT(seq, 58))
    return false;

  setRequired(seq, n, 0);
  if (!testSequenceSMPT(seq, 0))
    return false;

  return true;
}

bool testExtendedSequence0() {
  int n = 3;
  int durations[] = {2, 3, 4};
  int setups[] = {10, 5, 1};
  SequenceSMPT seq(n, durations, setups, true);
  setRequired(seq, n, 0);
  if (!testSequenceSMPT(seq, 0))
    return false;

  setRequired(seq, n, 1);
  if (!testSequenceSMPT(seq, 24))
    return false;

  setRequired(seq, 2 * n, 1);
  if (!testSequenceSMPT(seq, 64))
    return false;

  return true;
}

bool testSequence2() {
  int n = 4;
  int durations[] = {1, 2, 3, 4};
  int setups[] = {1, 1, 1, 1};
  SequenceSMPT seq(n, durations, setups);

  setRequired(seq, n, 1);
  seq.setRequired(1, 10);
  if (!testSequenceSMPT(seq, 107))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(2, 10);
  if (!testSequenceSMPT(seq, 188))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(3, 10);
  if (!testSequenceSMPT(seq, 260))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(4, 10);
  if (!testSequenceSMPT(seq, 323))
    return false;

  return true;
}

bool testSequence3(int n, int values[], int order[]) {
  SequenceSMPT seq(n, values, values);

  setRequired(seq, n, 1);
  seq.setRequired(order[1], 4);
  if (!testSequenceSMPT(seq, 54))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(order[2], 2);
  if (!testSequenceSMPT(seq, 44))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(order[3], 2);
  if (!testSequenceSMPT(seq, 49))
    return false;

  setRequired(seq, n, 1);
  seq.setRequired(order[4], 2);
  if (!testSequenceSMPT(seq, 55))
    return false;

  seq.setRequired(order[3], 2);
  seq.setRequired(order[4], 6);
  if (!testSequenceSMPT(seq, 209))
    return false;

  seq.setRequired(order[3], 3);
  seq.setRequired(order[4], 100);
  if (!testSequenceSMPT(seq, 22144))
    return false;
  
  
  return true;
}


bool testSequence3() {
  int n = 4;
  int v1[] = {1, 2, 3, 4};
  int p1[] = {0, 1, 2, 3, 4};
  if (!testSequence3(n, v1, p1)) return false;

  int v2[] = {4, 3, 2, 1};
  int p2[] = {0, 4, 3, 2, 1};
  if (!testSequence3(n, v2, p2)) return false;

  int v3[] = {4, 1, 3, 2};
  int p3[] = {0, 2, 4, 3, 1};
  if (!testSequence3(n, v3, p3)) return false;

  int v4[] = {3, 2, 4, 1};
  int p4[] = {0, 4, 2, 1, 3};
  if (!testSequence3(n, v4, p4)) return false;

  int v5[] = {2, 4, 1, 3};
  int p5[] = {0, 3, 1, 4, 2};
  if (!testSequence3(n, v5, p5)) return false;

  return true;
}

// bool testSequence3() {
//   int n = 4;
//   int durations[] = {1, 2, 3, 4};
//   int setups[] = {1, 2, 3, 4};
//   SequenceSMPT seq(n, durations, setups);

//   setRequired(seq, n, 1);
//   seq.setRequired(1, 4);
//   if (!testSequenceSMPT(seq, 54))
//     return false;

//   setRequired(seq, n, 1);
//   seq.setRequired(2, 2);
//   if (!testSequenceSMPT(seq, 44))
//     return false;

//   setRequired(seq, n, 1);
//   seq.setRequired(3, 2);
//   if (!testSequenceSMPT(seq, 49))
//     return false;

//   setRequired(seq, n, 1);
//   seq.setRequired(4, 2);
//   if (!testSequenceSMPT(seq, 55))
//     return false;

//   seq.setRequired(3, 2);
//   seq.setRequired(4, 6);
//   if (!testSequenceSMPT(seq, 209))
//     return false;

//   seq.setRequired(3, 3);
//   seq.setRequired(4, 100);
//   if (!testSequenceSMPT(seq, 22144))
//     return false;
  
  
//   return true;
// }

bool testSequence4() {
  int n = 3;
  int durations[] = {3, 2, 1};
  int setups[] = {1, 1, 3};
  SequenceSMPT seq(n, durations, setups);

  setRequired(seq, n, 1);
  if (!testSequenceSMPT(seq, 13))
    return false;

  return true;
}

int main() {
  //if (!testSequence3()) {
    if(!testSequence0() || !testSequence1() || !testSequence2() || !testSequence3() ||
    !testSequence4() || !testExtendedSequence0()) {
    std::cout << std::endl << "TEST FAILURE" << std::endl;
    return (EXIT_FAILURE);
  }
  std::cout << std::endl << "TEST SUCCESS" << std::endl;
  return (EXIT_SUCCESS);
}

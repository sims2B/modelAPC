#include "Solution.h"
#include <algorithm>
#include <vector>
#include <tuple>

#ifndef ALPHA
#define alpha 1
#endif

#ifndef BETA
#define beta 1
#endif

typedef int(*ptrHeur)(Problem,Solution&);
  

int RH(const Problem& P, Solution& s, ptrHeur);
int disqualify(Problem& Q, const std::vector<std::tuple<int,int>>& disqualif, const int& iter, const int& nbDisqualif);


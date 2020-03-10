#include "ilcConstraintAPC.h"

#include <algorithm>

// #define DEBUG

bool compareWeights(FamilyRun f1, FamilyRun f2)
{
  return f1.getWeight() < f2.getWeight();
}

ILCCTDEMON0(Relax1SFDemon, IlcRelax1SFConstraintI, varDemon)

void IlcRelax1SFConstraintI::post()
{
  IloCPEngine cp = getCPEngine();
  _f.whenRange(Relax1SFDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++)
  {
    _x[i].whenRange(Relax1SFDemon(cp, this));
  }
}

// set domains and count total number of required jobs
void IlcRelax1SFConstraintI::initSequence()
{
  for (int i = 1; i <= _n; i++)
  {
    s.setRequired(i, _x[i - 1].getMin());
  }
  s.sequencing();
}

void IlcRelax1SFConstraintI::reduceCardFamily(int i)
{
  const int min = _x[i - 1].getMin();
  int max = _x[i - 1].getMax();
  const int flow = _f.getMax();
  // std::cout << "FLOW " << flow << std::endl;
  while (max > min)
  {
    // std::cout << "MAX " << max << std::endl;
    s.setRequired(i, max);
    s.sequencing();
    if (s.searching(flow))
    {
      _x[i - 1].setMax(max);
      break;
    }
    max--;
  }
  s.setRequired(i, min);
}

void IlcRelax1SFConstraintI::initExtendedSequence()
{
  for (int i = 1; i <= _n; i++)
  {
    se.setRequired(i, _x[i - 1].getMin());
    se.setRequired(_n + i, 0);
  }
}

bool IlcRelax1SFConstraintI::extendSequence(int size)
{
  int f = 0;
  int missing = size - se.getSize();
#ifdef DEBUG
  std::cout << "MISSING " << missing << std::endl;
#endif
  while (missing > 0 && f < _n)
  {
    IlcIntVar x = _x[orderSPT[f] - 1];
    int opt = x.getMax() - x.getMin();
#ifdef DEBUG
    if (opt == 0)
      std::cout << "NO MORE FAMILY " << _n + orderSPT[f] << std::endl;
    else
      std::cout << "ADD FAMILY " << _n + orderSPT[f] << std::endl;
#endif
    if (opt > missing) {
      opt = missing;
    }
    se.setRequired(_n + orderSPT[f], opt);
    missing -= opt;
    f++;
  }
  return missing == 0;
}

void IlcRelax1SFConstraintI::reduceCardMachine()
{
  int f = _n - 1;
  const int flowUB = _f.getMax();
   se.sequencing();
   //std::cout << "COMPUTED LB " << se.searching() << " UB "<< flowUB << std::endl;

  while (!se.searching(flowUB))
  {
    while (f >= 0)
    {
      int i = _n + orderSPT[f];
    
      if (se.getRequired(i) > 0) {
        se.decrementRequired(i);
        break;
      }
        f--;
    }
    if (f >= 0) {
      se.sequencing();
     #ifdef DEBUG
      se.printSequence();
      printf("\n");
      #endif
    }
    else
      break;
  }
}

void IlcRelax1SFConstraintI::propagate()
{
  initSequence();
  // #ifdef DEBUG
  //   std::cout << "FLOWTIME " <<  _f;
  // #endif
  _f.setMin(s.searching());
  //   #ifdef DEBUG
  //   std::cout << " -> " <<  _f << std::endl;
  // #endif
  if (propagationMask & 2)
  {
    for (int i = 1; i <= _n; i++)
    {
      // const int oldMax = _x[i-1].getMax();
      if (!_x[i - 1].isFixed())
        reduceCardFamily(i);
      //  const int newMax = _x[i-1].getMax();
      //   if(newMax != oldMax) {
      //    std::cout << "N_F" <<  i << ": " << oldMax << " -> " << newMax <<
      //    std::endl;
      //  }
    }
  }
  if (propagationMask & 4)
  {
    // TODO trigger only if the card variable is not fixed ?
    initExtendedSequence();
   
    ///////////
    // Temporary code while waiting for the card variable
    int maxCard = 0;
    for (int i = 1; i <= _n; i++)
    {
      maxCard += _x[i - 1].getMax();
    }
#ifdef DEBUG
 std::cerr << std::endl
            << std::endl
            << "START" << std::endl;
    se.printSequence();
    std::cout << "MAX CARD " << maxCard << " FLOWTIME " << _f.getMax() << std::endl;
#endif
    ///////////
    if (extendSequence(maxCard))
    {
 #ifdef DEBUG
    se.printSequence();
#endif
reduceCardMachine();
#ifdef DEBUG
      std::cout << "NEW MAX CARD " << se.getSize() << std::endl;
#endif
 //if(maxCard > se.getSize()) 
 std::cout << "UPDATE MAX CARD " << maxCard << " -> "<< se.getSize() << " REAL MAX CARD " << _c.getMax()<< std::endl;
 //if(se.getSize() == 0) exit(EXIT_SUCCESS);
    }
    else
    {
      std::cerr << "Not enough optional jobs for the machine" << std::endl;
    }
  }
}

void IlcRelax1SFConstraintI::varDemon() { push(); }

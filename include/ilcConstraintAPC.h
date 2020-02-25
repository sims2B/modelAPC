#ifndef ILCCONSTRAINTAPC_H
#define ILCCONSTRAINTAPC_H

#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wclass-memaccess"

#ifndef __INTELLISENSE__ // code that generates an error squiggle #endif
// FIX : missing include generic.h 
#include <ilcp/cp.h>
#include <ilcp/cpext.h>
#endif
#include <vector>

// VFamily represents the assignment of jobs in the same family to a machine.
typedef struct VFamily {
  int index;
  int setup;
  int duration;
  int required;
  int optional; // to remove
  double weight;
} VFamily;

typedef struct Run {
  int start;
  int duration;
  int end;
  int pred;
  int next;
} Run;

typedef struct SequenceData {
  int makespan;
  int flowtime;
} SequenceData;

#define WMPT(f) ((double)f.duration + ((double)f.setup) / ((double)f.required))
#define FLOW(f) ((f.required * (f.required + 1) / 2)*f.duration)

typedef std::vector<Run> Schedule;
typedef std::vector<VFamily> Families;

// This is simply a constraint that is pushed and that calls the execute()
// function of the above custom inferencer
class IlcRelax1SFConstraintI : public IlcConstraintI {
 protected:
  IlcInt _n;
  IlcIntVarArray _x;
  IlcIntVar _f;
  //IlcIntArray _d;
  //IlcIntArray _s;
  Schedule s;
  Families f;

 public:
  IlcRelax1SFConstraintI(IloCPEngine cp, IlcIntVarArray families,
                         IlcIntVar flowtime, IlcIntArray durations,
                         IlcIntArray setups)
      : IlcConstraintI(cp),
        _n(families.getSize()),
        _x(families),
        _f(flowtime)
  //      _d(durations),
  //      _s(setups) 
  {
    s.reserve(_n + 1);
    f.reserve(_n + 1);

    // Create blocks and families
    for (int i = 0; i <= _n; i++) {
      s.push_back(Run{0, 0, 0, 0, 0});
    }

    f.push_back(VFamily{-1, 0, 0, 0, 0, 0});
    for (int i = 1; i <= _n; i++) {
      f.push_back(
          VFamily{i - 1, (int)setups[i - 1], (int)durations[i - 1], 0, 0, 0});
    }
  }

  ~IlcRelax1SFConstraintI() {}
  virtual void post();
  virtual void propagate();
  void varDemon();

 private:
  void initSWMPT();
  void scheduleEmptyRun(int i);
  int scheduleRun(int i, int setup);

  IloInt sequenceSWMPT();
};


#endif
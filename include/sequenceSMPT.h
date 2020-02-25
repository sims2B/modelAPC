#ifndef SEQUENCESMPT_H
#define SEQUENCESMPT_H

#include <cstdio>
#include <functional>
#include <iostream>
#include <vector>

#define SUM1N(n) ((n * (n + 1)) / 2)

// VFamily represents the assignment of jobs in the same family to a machine.
class FamilyRun {
  friend class SequenceSMPT;

 public:
  const int index;
  const int setup;
  const int duration;

 protected:
  int required;
  int slength;
  int length;
  double weight;

  int pred;
  int next;
  int endtime;
  int flowtime; // TODO Remove var

 public:
  FamilyRun() : FamilyRun(0, 0, 0) {}

  FamilyRun(int index, int setup, int duration)
      : index(index),
        setup(setup),
        duration(duration),
        required(0),
        slength(0),
        length(0),
        weight(0),
        pred(0),
        next(0),
        endtime(0),
        flowtime(0) {}

  int getRequired() { return required; }

  double getWeight() { return weight; }

  void print() {
    printf("RUN e=%-2d l=%-3d f=%-4d p=%-3d n=%-3d\t", endtime, length,
           flowtime, pred, next);
    printf("F%d[%d] d=%-2d s=%-2d w=%.2f\n", index, required, duration, setup,
           weight);
  }

 protected:
  int setRequired(int required) {
    int delta;
    if (required > 0) {
      delta = required - this->required;
      this->required = required;
      slength = setup;
      length = setup + required * duration;
      flowtime = SUM1N(required) * duration;
      weight = ((double)duration + ((double)setup) / ((double)required));
    } else {
      delta = - this->required;
      this->required = 0;
      slength = 0;
      length = 0;
      flowtime = 0;
      weight = 0;
    }
    return delta;
  }

  void initFirstRun(int n) { this->next = n; }

  void scheduleAfter(FamilyRun* run) {
    next = run->next - required;
    pred = run->pred + run->required;
    endtime = run->endtime + length;
  }
  
  int getFlowtime() {return required * slength + flowtime + next * length;}

  int cancelSetup() { return -(required + next) * slength; }

  int moveFirst() {
    int delta = pred * (length - slength); // predecessors are postponed
    delta -= required * (endtime - length); // the current run is moved first
    delta -= (required + next) * slength; //  the setup of the current following runs are removed 
    return delta;
  }
};

typedef std::vector<FamilyRun> Sequence;
typedef std::vector<FamilyRun*> Runs;

class SequenceSMPT {
 public:
  const int n;
  int nreq;
  int flowtimeWS;

 protected:
  std::vector<FamilyRun*> sequence;
  std::vector<FamilyRun*> runs;

 public:
  SequenceSMPT(int n, int durations[], int setups[]) : n(n), nreq(0) {
    FamilyRun* run = new FamilyRun();
    sequence.push_back(run);
    runs.push_back(run);
    for (int i = 0; i < n; i++) {
      sequence.push_back(new FamilyRun(i + 1, setups[i], durations[i]));
      runs.push_back(sequence[i + 1]);
    }
  }

  ~SequenceSMPT() {
    for (int i = 0; i <= n; i++) {
      delete (sequence[i]);
    }
  }

  void setRequired(int family, int required) {
    nreq += runs[family]->setRequired(required);
  }

  void sequencing();

  int searching();

  bool searching(int flowtimeUB);

  void printSequence() {
    for (auto run : sequence) {
      run->print();
    }
  }

 private:
  int searchNextRun(int from);
};

#endif
#ifndef SEQUENCESMPT_H
#define SEQUENCESMPT_H

#include <cstdio>
#include <iostream>
#include <functional>
#include <vector>

#define SUM1N(n) ( (n * (n + 1)) / 2)

// VFamily represents the assignment of jobs in the same family to a machine.
class FamilyRun {

  friend class SequenceSMPT; 

  public :
  const int index;
  const int setup;
  const int duration;

  protected:
  int required;
  int length;
  double weight;
  
  int pred;
  int next;
  int endtime;
  int flowtime;

  public:
  FamilyRun(int index, int setup, int duration) : 
    index(index), setup(setup), duration(duration), 
    required(0), length(0), weight(0), pred(0), next(0), endtime(0), flowtime(0) {
  } 

  double getWeight() {return weight;}
  
  int getEndtime() {return endtime;}

  int getFlowtime() {return flowtime;}

  void setRequired(int required) {
    this->required = required;
    if(required > 0) {
      weight = ((double) duration + ( (double) setup) / ((double) required));
      length = setup + required * duration;
    } else {
      weight = 0;
      length = 0;
    }
  }

void print() {
       printf("F%d[%d] d=%-2d s=%-2d w=%.2f\n", index, required, duration, setup, weight);
       printf("RUN e=%-2d l=%-3d f=%.3d\n", endtime, length, flowtime);
 }

  protected:

  void scheduleAfter(FamilyRun* run) {
    next = run->next - required;
    pred = run->pred + run->required; 
    endtime = run->endtime + length;
    flowtime = required * setup + SUM1N(required) * duration + next * length;
  }

  
};

typedef std::vector<FamilyRun> Sequence;
typedef std::vector<FamilyRun*> Runs;

class SequenceSMPT {
  public:
  const int n;
  protected:
  std::vector<FamilyRun*> sequence;
  std::vector<FamilyRun*> runs;
  
  public:
  SequenceSMPT(int n, int durations[], int setups[]) : n(n) {
    FamilyRun* run = new FamilyRun(0, 0, 0);
    sequence.push_back(run);
    runs.push_back(run);
    for (int i = 0; i < n; i++) {
        sequence.push_back(new FamilyRun(i + 1, setups[i], durations[i]));
        runs.push_back(sequence[i + 1]);
    }
  }

  ~SequenceSMPT() {
    for (int i = 0; i <= n; i++) {
        delete(sequence[i]);
    }
  }
 
  void setRequired(int family, int required) {
    runs[family]->setRequired(required);
  }
  
  void sequencing();

  void printSequence() {
    for (auto run : sequence) {
      run->print();
    }
  }


};


#endif
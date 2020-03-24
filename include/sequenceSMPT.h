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
  int flowtime; 
  
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
    int delta = -this->required;
      if (required > 0) {
      delta += required;
      this->required = required;
      slength = setup;
      length = setup + required * duration;
      flowtime = SUM1N(required) * duration;
      weight = ((double)duration + ((double)setup) / ((double)required));
    } else {
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

  int getFlowtime() { return required * slength + flowtime + next * length; }

  int cancelSetup() { return -(required + next) * slength; }

  int moveFirst() {
    int delta = pred * (length - slength);   // predecessors are postponed
    delta -= required * (endtime - length);  // the current run is moved first
    delta -= (required + next) *
             slength;  //  the setup of the current following runs are removed
    return delta;
  }
};

typedef std::vector<FamilyRun> Sequence;
typedef std::vector<FamilyRun*> Runs;

class SequenceSMPT {
 public:
  // Number of runs 
  const int n;
  
 protected:
  /**
   * @brief the total number of jobs in the runs
   * 
   */
  int size; 
  
  /**
   * @brief the flowtime of the runs in SMPT order with an initial setup 
   */
  int flowtimeWS;  

  /**
   * @brief the runs in SMPT order
   * 
   * The sequence starts by a dummy run with no jobs, nil duration, and nil setup. 
   */
  std::vector<FamilyRun*> sequence;
  /**
   * @brief the runs by family index starting at 1.
   * 
   */
  std::vector<FamilyRun*> runs;

 public:
  SequenceSMPT(std::vector<int> durations, std::vector<int> setups,
               bool extended)
      : n( (extended + 1) * durations.size()), size(0) {
    const int n = durations.size();
     sequence.reserve(this->n + 1);
    runs.reserve(this->n + 1);
    FamilyRun* run = new FamilyRun();
    sequence.push_back(run);
    for (int i = 0; i < n; i++) {
      sequence.push_back(new FamilyRun(i + 1, setups[i], durations[i]));
    }
    if (extended) {
      for (int i = 0; i < n; i++) {
        sequence.push_back(new FamilyRun(n + i + 1, 0, durations[i]));
      }
    }
    // Copying vector by copy function
    copy(sequence.begin(), sequence.end(), back_inserter(runs));
  }
  
  SequenceSMPT(std::vector<int> durations, std::vector<int> setups)
      : SequenceSMPT(durations, setups, false) {}

/**
 * @brief Construct a new sequence of runs for the given families.
 * 
 * @param n the number of family
 * @param durations the family durations
 * @param setups the family setups
 * @param extended if true, append additional runs with the same durations, but nil setups. 
 */
  SequenceSMPT(int n, int durations[], int setups[], bool extended)
      : n((extended +1) * n), size(0) {
    sequence.reserve(this->n + 1);
    runs.reserve(this->n + 1);
    FamilyRun* run = new FamilyRun();
    sequence.push_back(run);
    for (int i = 0; i < n; i++) {
      sequence.push_back(new FamilyRun(i + 1, setups[i], durations[i]));
    }
    if (extended) {
      for (int i = 0; i < n; i++) {
        sequence.push_back(new FamilyRun(n + i + 1, 0, durations[i]));
      }
    }
    copy(sequence.begin(), sequence.end(), back_inserter(runs));
  }

  SequenceSMPT(int n, int durations[], int setups[])
      : SequenceSMPT(n, durations, setups, false) {}

  ~SequenceSMPT() {
    for (int i = 0; i <= n; i++) {
      delete (sequence[i]);
    }
  }
  /**
   * @brief Get the total number of jobs in the runs
   * 
   */
  int getSize() {return size;}
  
  /**
   * @brief Get the number of required jobs for a family 
   * 
   * @param family the family index starting at 1.
   * @return int number of required jobs
   */
  int getRequired(int family) {
    return runs[family]->getRequired();
  }

  /**
   * @brief Set the number of required jobs for a family.
   * 
   * @param family the family index starting at 1.
   * @param required number of required jobs.
   * 
   */
  void setRequired(int family, int required) {
    size += runs[family]->setRequired(required);
  }

  /**
   * @brief Decrement the number of required jobs for a family.
   * 
   * @param family the family index starting at 1.
  */
  void decrementRequired(int family) {
     setRequired(family, getRequired(family) - 1);
  }


  int searching();

  bool searching(int flowtimeUB);

  void printSequence() {
    for (auto run : sequence) {
      run->print();
    }
  }
 int sequencing();
 private:
  
  int searchNextRun(int from);
};

#endif

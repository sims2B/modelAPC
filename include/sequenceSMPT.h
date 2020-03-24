#ifndef SEQUENCESMPT_H
#define SEQUENCESMPT_H

#include <cstdio>
#include <functional>
#include <iostream>
#include <vector>

#define SUM1N(n) ((n * (n + 1)) / 2)

/**
 * @brief This class represents a run : the assignment of jobs in the same
 * family to the same machine. It contains information about the family as well
 * as many precomputed values useful for moving efficiently a run in the
 * sequence. Note that precomputations differ between empty and non empty runs.
 */
class FamilyRun {
  friend class SequenceSMPT;

 public:
  /**
   * @brief Index of the family starting at 1
   *
   */
  const int index;
  /**
   * @brief Setup of the family
   *
   */
  const int setup;
  /**
   * @brief Duration of the family.
   *
   */
  const int duration;

 protected:
  /**
   * @brief Number of jobs in the run
   */
  int required;
  /**
   * @brief the total duration of the run including the setup
   */
  int slength;
  /**
   * @brief the duration of the run without setup
   */
  int length;
  /**
   * @brief the SMPT weight of the run used for sorting
   */
  double weight;
  /**
   * @brief number of predecessors (jobs)
   */
  int pred;
  /**
   * @brief number of successors (jobs)
   */
  int next;
  /**
   * @brief end time of the run
   */
  int endtime;
  /**
   * @brief the internal flowtime of the run
   */
  int flowtime;

 public:
  FamilyRun() : FamilyRun(0, 0, 0) {}

  /**
   * @brief Construct a new empty run for the given family.
   */
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
 /**
  * @brief Set the number of required jobs in the run.
  * This differs for empty and non empty runs.
  * @param required the new number of jobs
  * @return int the difference between the actual and previous numbers of jobs
  */
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

  /**
   * @brief tet the number of successors (job).
   * This method is only used to initialize the first dummy run of a sequence.
   * 
   * @param next the total number of jobs in the sequence
   */
  void setNext(int next) { this->next = next; }
  
  /**
   * @brief schedule this run immediatly after the given run. 
   * 
   * @param run the immediate predecessor of this run
   */
  void scheduleAfter(FamilyRun* run) {
    next = run->next - required;
    pred = run->pred + run->required;
    endtime = run->endtime + length;
  }
  
  /**
   * @brief get the flowtime induced only by this run 
   * 
   * @return int 
   */
  int getFlowtime() { return required * slength + flowtime + next * length; }

  /**
   * @brief Get the flowtime variation if there is no setup time.
   * 
   * @return int the flowtime decrease if there is no setup
   */
  int cancelSetup() { return -(required + next) * slength; }

  /**
   * @brief Get the flowtime variation if this run is moved first.
   * This run is scheduled first without setup while the rest of the sequence remains unchanged.
   * 
   * @return int the flowtime variation
   */
  int moveFirst() {
    int delta = pred * (length - slength);   // predecessors are postponed
    delta -= required * (endtime - length);  // the current run is moved first
    delta -= (required + next) *
             slength;  //  the setup of the current following runs are removed
    return delta;
  }
};

/**
 * @brief this class optimally sequences jobs with family setups.
 * There is no initial setup.
 * To do so, it first sequences optimally the jobs with an initial setup.
 * Then, it computes the best sequence among those where a non empty run is moved first without initial setup.   
 * 
 */
class SequenceSMPT {
 public:
  /**
   * @brief bumber of runs
   *
   */
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
   * The sequence starts by a dummy run with no jobs, nil duration, and nil
   * setup.
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
      : n((extended + 1) * durations.size()), size(0) {
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
   * @param extended if true, append additional runs with the same durations,
   * but nil setups.
   */
  SequenceSMPT(int n, int durations[], int setups[], bool extended)
      : n((extended + 1) * n), size(0) {
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
  int getSize() { return size; }

  /**
   * @brief Get the number of required jobs for a family
   *
   * @param family the family index starting at 1.
   * @return int number of required jobs
   */
  int getRequired(int family) { return runs[family]->getRequired(); }

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

  /**
   * @brief search the optimal flowtime of the sequence without initial setups
   *
   * @return int the optimal flowtime
   */
  int searching();

  /**
   * @brief search a sequence without initial setups under a given upper bound
   * on the flowtime
   *
   * @param flowtimeUB the upper bound for the sequence flowtime
   * @return true at least one feasible sequence exists
   * @return false no feasible sequence exists
   */
  bool searching(int flowtimeUB);

  /**
   * @brief print the sequence in a human readable format (useful for
   * debugging).
   *
   */
  void printSequence() {
    for (auto run : sequence) {
      run->print();
    }
  }

 private:
  /**
   * @brief sequence the runs in SMPT order with initial setups.
   * This requires sorting the runs in SMPT order.
   *
   * @return int
   */
  int sequencing();

  /**
   * @brief Search the next non empty run.
   *
   * @param from from the given index (not inclusive).
   * @return int the index of the next non empty run in the sequence.
   */
  int searchNextRun(int from);
};

#endif

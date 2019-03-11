#include "heuristics.h"
#include <algorithm>

int ListHeuristic::chooseFamily(int m) {
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m) && problem.getNbJobs(f)) {
      if (selected == -1)
        selected = f;
      else if (problem.getThreshold(f) < problem.getThreshold(selected))
        selected = f;
      else if (problem.getThreshold(f) == problem.getThreshold(selected) &&
               problem.getDuration(f) < problem.getDuration(selected))
        selected = f;
    }
  }
  return selected;
}

void ListHeuristic::schedule(const int m, const int f) {
  int endMch = solution.getEnd(m);
  // ajout d'un job de f sur la machine
  (solution.getLastJob(m).getFam() == f
       ? solution.addJob(Job(f, endMch, 1), m)
       : solution.addJob(Job(f, endMch + problem.getSetup(f), 1), m));

  problem.decreaseNbJobs(f);

  updateDisqualif();
}

void ListHeuristic::updateDisqualif() {
  for (int m = 0; m < problem.getNbMchs(); ++m) {
    for (int f = 0; f < problem.getNbFams(); ++f) {
      if (problem.isQualif(f, m)) {
        int lastOf = solution.lastOf(f, m).getStart();
        if (lastOf == -1) lastOf = 0;
        if (solution.getEnd(m) - lastOf > problem.getThreshold(f)) {
          solution.setDisqualif(lastOf + problem.getThreshold(f), f, m);
          problem.disqualif(f, m);
        }
      }
    }
  }
}

void ListHeuristic::doSolve() {
  // init and variable
  Problem save(problem);
  const int M = problem.getNbMchs();
  int i, f;

  // algo
  i = 0;

  while (i < problem.getNbJobs()) {
    bool feasible = false;
    for (int j = 0; j < M; ++j) {
      if ((f = chooseFamily(j)) != -1) {
        schedule(j, f);
        feasible = true;
        i++;
      }
    }
    if (feasible == false) {
      return;
    }
  }
  problem = save;
  setSAT();
}

void SchedCentricHeuristic::schedule(const int m, const int f) {
  int endMch = solution.getEnd(m);
  // ajout d'un job de f sur la machine
  (solution.getLastJob(m).getFam() == f
       ? solution.addJob(Job(f, endMch, 1), m)
       : solution.addJob(Job(f, endMch + problem.getSetup(f), 1), m));

  problem.decreaseNbJobs(f);

  updateDisqualif();
}

void SchedCentricHeuristic::updateDisqualif() {
  for (int m = 0; m < problem.getNbMchs(); ++m) {
    // update disqualification
    for (int f = 0; f < problem.getNbFams(); ++f) {
      if (problem.isQualif(f, m)) {
        int lastOf = solution.lastOf(f, m).getStart();
        if (lastOf == -1) lastOf = 0;
        if (solution.getEnd(m) - lastOf > problem.getThreshold(f)) {
          solution.setDisqualif(lastOf + problem.getThreshold(f), f, m);
          problem.disqualif(f, m);
        }
      }
    }
  }
}

int SchedCentricHeuristic::remainingThresh(const int &f, const int &m,
                                           const int &t) {
  return solution.getDisqualif(f, m) + problem.getThreshold(f) - t;
}

int SchedCentricHeuristic::chooseFamily(const int &m) {
  int current = solution.getLastJob(m).getFam();
  int critical, selected = -1;
  // if there's no task on m or no job of current to schedule
  // choose with the minimum threshold rule
  if (solution.getNbJobsOn(m) == 0 || problem.getNbJobs(current) == 0)
    selected = famWithMinThresh(m, solution.getEnd(m));
  else {
    // if scheduled current does not produced a disqualification
    // schedule a job of current
    selected = current;
    if ((critical = famWithMinThresh(m, solution.getEnd(m))) != -1)
      if (remainingThresh(
              critical, m,
              solution.getEnd(
                  m)) -  // if scheduled selected => disqulification of critical
              problem.getSetup(critical) <
          problem.getDuration(selected))
        selected = critical;  // then schedule critical
  }
  return selected;
}

int SchedCentricHeuristic::famWithMinThresh(const int &m, const int &t) {
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m) && problem.getNbJobs(f) != 0) {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m, t) < remainingThresh(selected, m, t))
        selected = f;
      else if (remainingThresh(f, m, t) == remainingThresh(selected, m, t) &&
               problem.getDuration(f) < problem.getDuration(selected))
        selected = f;
    }
  }
  return selected;
}

void SchedCentricHeuristic::doSolve() {
  Problem save(problem);
  const int M = problem.getNbMchs();
  int i, f, j;

  i = 0;
  while (i < problem.getNbJobs()) {
    bool feasible = false;
    for (j = 0; j < M; ++j) {
      if ((f = chooseFamily(j)) != -1) {
        feasible = true;
        schedule(j, f);
        ++i;
      }
    }

    if (!feasible) return;
  }
  problem = save;
  setSAT();
}

void QualifCentricHeuristic::doSolve() {
  // FIXME REALLY NOT SURE ABOUT THIS ONE !
  if (findSchedule()) {
    intraChange();
    interChange();
    setSAT();
  }
}

void QualifCentricHeuristic::schedule(const int m, const int f) {
  int endMch = solution.getEnd(m);
  // ajout d'un job de f sur la machine
  (solution.getLastJob(m).getFam() == f
       ? solution.addJob(Job(f, endMch, 1), m)
       : solution.addJob(Job(f, endMch + problem.getSetup(f), 1), m));

  problem.decreaseNbJobs(f);

  updateDisqualif();
}

void QualifCentricHeuristic::updateDisqualif() {
  for (int m = 0; m < problem.getNbMchs(); ++m) {
    // update disqualification
    for (int f = 0; f < problem.getNbFams(); ++f) {
      if (problem.isQualif(f, m)) {
        int lastOf = solution.lastOf(f, m).getStart();
        if (lastOf == -1) lastOf = 0;
        if (solution.getEnd(m) - lastOf > problem.getThreshold(f)) {
          solution.setDisqualif(lastOf + problem.getThreshold(f), f, m);
          problem.disqualif(f, m);
        }
      }
    }
  }
}

int QualifCentricHeuristic::remainingThresh(const int &f, const int &m,
                                            const int &t) {
  return solution.getDisqualif(f, m) + problem.getThreshold(f) - t;
}

int QualifCentricHeuristic::findSchedule() {
  Problem save(problem);
  int i, f, j;

  i = 0;
  while (i < problem.getNbJobs()) {
    bool feasible = false;
    for (j = 0; j < problem.getNbMchs(); ++j) {
      if ((f = chooseFamily(j)) != -1) {
        feasible = true;
        schedule(j, f);
        ++i;
      }
    }

    if (!feasible) return 0;
  }
  problem = save;
  return 1;
}

int QualifCentricHeuristic::chooseFamily(const int &m) {
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m) && problem.getNbJobs(f) != 0) {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m, solution.getEnd(m)) <
               remainingThresh(selected, m, solution.getEnd(m)))
        selected = f;
      else if (remainingThresh(f, m, solution.getEnd(m)) ==
                   remainingThresh(selected, m, solution.getEnd(m)) &&
               problem.getDuration(f) < problem.getDuration(selected))
        selected = f;
    }
  }
  return selected;
}

void QualifCentricHeuristic::intraChange() {
  const int F = problem.getNbFams();
  for (int k = 0; k < problem.getNbMchs(); ++k) {  // sur chaque machine
    if (solution.getNbJobsOn(k) > 2) {
      std::vector<int> moved(
          F, 0);  // le nombre max de chgmt pour une famille est sa taille

      // intraChange movement
      bool update;
      do {
        update = false;
        Job j = solution.getLastJob(k);  // recupere le dernier job schedule
        Job i = solution.firstOf(j.getFam(),
                                 k);  // et le premier job de la meme famille
        if (!(i == j) && moved[j.getFam()] < problem.getNbJobs(j.getFam()) &&
            !addDisqualif(i, j, k)) {
          update = true;
          moved[j.getFam()]++;
          updateTime(i, j, k);
        }
      } while (update);
    }
  }
  // comme on a updater le cmax, on a pu supprimer des disqualifications => maj
  int Cmax = solution.getMaxEnd();
  for (int f = 0; f < problem.getNbFams(); ++f)
    for (int i = 0; i < problem.getNbMchs(); ++i) {
      if (solution.getDisqualif(f, i) >= Cmax)
        solution.setDisqualif(std::numeric_limits<int>::max(), f, i);
    }
}

int QualifCentricHeuristic::addDisqualif(const Job &i, const Job &j,
                                         const int &m) {
  const int F = problem.getNbFams();

  for (int f = 0; f < F; ++f) {
    if (f != i.getFam() && problem.isQualif(f, m)) {
      Job firstOcc = solution.firstOf(f, m);
      if (firstOcc.getStart() != -1) {
        if (firstOcc.getStart() > i.getStart()) {
          if (firstOcc.getStart() > problem.getThreshold(f)) return 1;
        } else {
          Job nextOcc = solution.nextOf(firstOcc, f, m);
          if (nextOcc.getStart() - firstOcc.getStart() > problem.getThreshold(f))
            return 1;
        }
      }
    }
  }
  return 0;
}

void QualifCentricHeuristic::updateTime(const Job &i, const Job &j,
                                        const int &k) {
  // update data for j, save j and remove j from the schedule
  Job j2(j.getFam(), i.getStart() + problem.getDuration(j.getFam()), j.getIndex());
  solution.removeLastJob(k);

  // shift all task after i by p_f(j)
  for (int l = 0; l < solution.getNbJobsOn(k); ++l) {
    Job cur = solution.getJobs(l, k);
    if (cur.getStart() > i.getStart()) {
      cur.shift(problem.getDuration(j2));
    }
  }

  // add updated j to the schedule
  solution.addJob(j2, k);
}

void QualifCentricHeuristic::interChange() {
  int k, nbJobs;
  Job deb(-1), fin(-1);

  for (k = 0; k < problem.getNbMchs(); ++k) {  // pour chaque machine
    if (solution.getNbJobsOn(k) > 1) {
      bool update;
      do {  // tant qu'on fait un move
        update = false;
        getLastGroup(deb, fin, k, nbJobs);
        int machineSelected = -1;
        int jobSelected = -1;
        findJobMachineMatch(k, deb, fin, machineSelected, jobSelected, nbJobs);

        if (machineSelected != -1 && jobSelected != -1) {
          update = true;
          updateTime(jobSelected, deb, nbJobs, k, machineSelected);
        }
      } while (update);
    }
  }

  int Cmax = solution.getMaxEnd();
  for (int f = 0; f < problem.getNbFams(); ++f)
    for (int i = 0; i < problem.getNbJobs(); ++i) {
      if (solution.getDisqualif(f, i) >= Cmax)
        solution.setDisqualif(std::numeric_limits<int>::max(), f, i);
    }
}

void QualifCentricHeuristic::findJobMachineMatch(int k, const Job &deb,
                                                 const Job &fin,
                                                 int &machineSelected,
                                                 int &jobSelected, int nbJobs) {
  for (int m = 0; m < problem.getNbMchs(); ++m) {
    if (problem.isQualif(fin.getFam(), m) &&
        k != m) {  // sur quelle machine je la met...
      for (int i = 0; i < solution.getNbJobsOn(m); ++i) {
        Job jobi = solution.getJobs(i, m);
        if (jobi.getFam() == fin.getFam())  // et avec quelle tache
          if (!addDisqualif(deb, jobi, m, k, nbJobs) &&
              !addCompletion(jobi, nbJobs, k, m))
            if (machineSelected == -1 ||
                solution.getEnd(m) < solution.getEnd(machineSelected)) {
              jobSelected = i;
              machineSelected = m;
            }
      }
    }
  }
}

// i sur machine m ; j sur machine k
int QualifCentricHeuristic::addDisqualif(const Job &deb, const Job &jobi,
                                         const int &m, const int &k,
                                         int nbJobs) {
  const int F = problem.getNbFams();

  // adding jobs [deb,fin] on m add a disqualif?
  for (int f = 0; f < F; ++f) {
    if (f != jobi.getFam() && problem.isQualif(f, m)) {
      Job firstOcc = solution.firstOf(f, m);

      if (firstOcc.getStart() != -1) {
        if (firstOcc.getStart() > jobi.getStart()) {
          if (firstOcc.getStart() + problem.getDuration(deb) * nbJobs >
              problem.getThreshold(f))
            return 1;
        }

        else {
          Job nextOcc = solution.nextOf(firstOcc, f, m);
          if (nextOcc.getStart() > jobi.getStart() &&
              nextOcc.getStart() + problem.getDuration(deb) * nbJobs -
                      firstOcc.getStart() >
                  problem.getThreshold(f))
            return 1;
        }
      }
    }
  }

  // removing [deb,fin] on k add a disqualif?
  Job prev = solution.getPreviousOcc(deb, deb.getFam(), k);
  // compute future cmax
  int cmax = 0;
  for (int i = 0; i < problem.getNbMchs(); ++i) {
    if (i == k && cmax < solution.getEnd(k) - nbJobs * problem.getDuration(deb))
      cmax = solution.getEnd(k) - nbJobs * problem.getDuration(deb);
    if (i == m && cmax < solution.getEnd(m) + nbJobs * problem.getDuration(deb))
      cmax = solution.getEnd(m) + nbJobs * problem.getDuration(deb);
    else if (cmax < solution.getEnd(i))
      cmax = solution.getEnd(i);
  }
  if (cmax - prev.getStart() > problem.getThreshold(deb)) return 1;
  return 0;
}

int QualifCentricHeuristic::addCompletion(const Job &i, const int &nbJobs,
                                          const int &k, const int &m) {
  return (solution.getEnd(k) <=
          solution.getEnd(m) + nbJobs * problem.getDuration(i));
}

void QualifCentricHeuristic::getLastGroup(Job &deb, Job &fin, int k,
                                          int &nbJobs) {
  fin=solution.getLastJob(k);
  uint i = solution.getNbJobsOn(k);
  while (i >= 0 && solution.getJobs(i, k).getFam() == fin.getFam()) --i;
  deb = solution.getJobs(i + 1, k);
  nbJobs = solution.getNbJobsOn(k) - 1 - i + 1;
}

void QualifCentricHeuristic::updateTime(const Job &i, const Job &deb,
                                        int nbJobs, int k, int m) {
  int j;
  // remove nbJobs last Job on k
  for (j = 0; j < nbJobs; ++j) solution.removeLastJob(k);

  // add nbJobs jobs of family f(i) after i and shift all jobs after
  // 1/ shift every job after i
  j = 0;
  while (!(solution.getJobs(j, m) == i)) j++;
  while (j < solution.getNbJobsOn(m)) {
    solution.getJobs(j, m).shift(nbJobs * problem.getDuration(i));
  }
  // 2/ add nbJobs of f(i) after i
  Job curr = i;
  for (j = 0; j < nbJobs; ++j) {
    Job temp(problem.getFam(i), curr.getStart() + problem.getDuration(i), -1);
    solution.addJob(temp, m);
    curr = temp;
  }
}

HeuristicAPC *makeHeuristic(Problem &problem, ConfigAPC &config,
                            std::string name) {
  if (name == H_LIST)
    return new ListHeuristic(problem, config);
  else if (name == H_QUALIF)
    return new QualifCentricHeuristic(problem, config);
  else
    return new SchedCentricHeuristic(problem, config);
}
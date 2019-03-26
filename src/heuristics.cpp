#include "heuristics.h"
#include <algorithm>
#include <cassert>

HeuristicAPC::HeuristicAPC(Problem &problem, ConfigAPC &config)
    : AbstractSolverAPC(problem, config) {
  assert(config.getObjectiveFunction() != "LEX" &&
         "Lexical objective not compatible with heuristics");
  if (config.getObjectiveFunction() == "MONO") {
    if (config.getWeightFlowtime() > config.getWeightQualified()) {
      alpha = 1;
      beta = 0;
    } else {
      beta = 1;
      alpha = 0;
    }
  } else {
    alpha = 1;
    if (config.getWeightFlowtime() > config.getWeightQualified())
      beta = 1;
    else
      beta = problem.computeHorizon() * problem.getNbJobs();
  }
}


void HeuristicAPC::schedule(const int f, const int m) {
  int endMch = solution.getEnd(m);
  // ajout d'un job de f sur la machine
  if (!solution.getNbJobsOn(m))
    solution.addJob(Job(f, 0, 1), m);
  else
    (solution.getLastJob(m).getFam() == f
         ? solution.addJob(Job(f, endMch, 1), m)
         : solution.addJob(Job(f, endMch + problem.getSetup(f), 1), m));

  problem.decreaseNbJobs(f);

  updateDisqualifLocal(m);
}

void HeuristicAPC::updateDisqualifLocal(int m) {
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m)) {
      int lastOf = solution.lastOf(f, m).getStart();

      if (lastOf == -1) lastOf = 0;
      if (solution.getEnd(m) + problem.getSetup(f) - lastOf >
          problem.getThreshold(f)) {
        solution.setDisqualif(lastOf + problem.getThreshold(f), f, m);
        problem.disqualif(f, m);
      }
    }
  }
}

void HeuristicAPC::updateDisqualifGlobal() {
  int cmax = solution.getMaxEnd();
  for (int m = 0; m < problem.getNbMchs(); ++m) {
    for (int f = 0; f < problem.getNbFams(); ++f) {
      if (problem.isQualif(f, m)) {
        int lastOf = solution.lastOf(f, m).getStart();

        if (lastOf == -1) lastOf = 0;
        if (cmax - lastOf > problem.getThreshold(f))
          solution.setDisqualif(lastOf + problem.getThreshold(f), f, m);
      }
    }
  }
}

int ListHeuristic::chooseFamily(int m) const {
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

void ListHeuristic::doSolve() {
  // init and variable
  Problem save(problem);
  const int M = problem.getNbMchs();
  int i, f;
  i = 0;
  std::cout << "*************resolution LH\n";

  // algo
  while (i < problem.getNbJobs()) {
    bool feasible = false;
    for (int j = 0; j < M; ++j) {
      if ((f = chooseFamily(j)) != -1) {
        schedule(f, j);
        feasible = true;
        i++;
      }
    }
    if (feasible == false) {
      return;
    }
  }
  updateDisqualifGlobal();
  problem = save;
  setSAT();
}

int SchedCentricHeuristic::remainingThresh(const int &f, const int &m) const {
  int lastStart = solution.lastOf(f, m).getStart();
  if (lastStart == -1)
    return problem.getThreshold(f) - solution.getEnd(m);
  else
    return lastStart + problem.getThreshold(f) - solution.getEnd(m);
}

int SchedCentricHeuristic::chooseFamily(const int &m) const {
  int current = solution.getLastJob(m).getFam();
  int critical, selected = -1;
  // if there's no task on m or no job of current to schedule
  // choose with the minimum threshold rule
  if (solution.getNbJobsOn(m) == 0 || problem.getNbJobs(current) == 0)
    selected = famWithMinThresh(m);
  else {
    // if scheduled current does not produced a disqualification
    // schedule a job of current
    selected = current;
    if ((critical = famWithMinThresh(m)) != -1)
      if (remainingThresh(
              critical,
              m) -  // if scheduled selected => disqulification of critical
              problem.getSetup(critical) <
          problem.getDuration(selected))
        selected = critical;  // then schedule critical
  }
  return selected;
}

int SchedCentricHeuristic::famWithMinThresh(const int &m) const {
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m) && problem.getNbJobs(f) != 0) {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m) < remainingThresh(selected, m))
        selected = f;
      else if (remainingThresh(f, m) == remainingThresh(selected, m) &&
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

  std::cout << "*************resolution SCH\n";
  i = 0;
  while (i < problem.getNbJobs()) {
    bool feasible = false;
    for (j = 0; j < M; ++j) {
      if ((f = chooseFamily(j)) != -1) {
        feasible = true;
        schedule(f, j);
        ++i;
      }
    }

    if (!feasible) return;
  }
  updateDisqualifGlobal();
  problem = save;
  setSAT();
}

void QualifCentricHeuristic::doSolve() {
  // FIXME REALLY NOT SURE ABOUT THIS ONE !

  std::cout << "*************resolution QCH\n";
  if (findSchedule()) {
    /*  std::cout << "\n after phase 1 \\\\ \n";
     solution.toTikz();
     std::cout << solution.toString(); */
    intraChange(); /*
     std::cout << "\n after phase 2 \\\\ \n";
     solution.toTikz();
     std::cout << solution.toString(); */
    interChange(); /*
     std::cout << "\n after phase 3 \\\\ \n";
     solution.toTikz();
     std::cout << solution.toString(); */
    setSAT();
  }
}

int QualifCentricHeuristic::remainingThresh(const int &f, const int &m) const {
  int lastStart = solution.lastOf(f, m).getStart();
  if (lastStart == -1)
    return problem.getThreshold(f) - solution.getEnd(m);
  else
    return lastStart + problem.getThreshold(f) - solution.getEnd(m);
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
        schedule(f, j);
        ++i;
      }
    }

    if (!feasible) return 0;
  }
  updateDisqualifGlobal();
  problem = save;
  return 1;
}

int QualifCentricHeuristic::chooseFamily(const int &m) const {
  int selected = -1;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    if (problem.isQualif(f, m) && problem.getNbJobs(f) != 0) {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m) < remainingThresh(selected, m))
        selected = f;
      else if (remainingThresh(f, m) == remainingThresh(selected, m) &&
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
      std::vector<int> nbMove(
          F, 0);  // le nombre max de chgmt pour une famille est sa taille

      // intraChange movement
      bool update;
      do {
        update = false;
        Job j = solution.getLastJob(k);  // recupere le dernier job schedule
        Job i = solution.firstOf(j.getFam(),
                                 k);  // et le premier job de la meme famille
        if (!(i == j) && nbMove[j.getFam()] < problem.getNbJobs(j.getFam()) &&
            !addDisqualif(i, j, k)) {
          update = true;
          nbMove[j.getFam()]++;
          updateTime(i, j, k);
        }
      } while (update);
    }
  }
  // comme on a updater le cmax, on a pu supprimer des disqualifications => maj
  int Cmax = solution.getMaxEnd();
  for (int f = 0; f < F; ++f)
    for (int i = 0; i < problem.getNbMchs(); ++i) {
      if (solution.getDisqualif(f, i) >= Cmax)
        solution.setDisqualif(std::numeric_limits<int>::max(), f, i);
    }
}

// j move next to i
int QualifCentricHeuristic::addDisqualif(const Job &i, const Job &j,
                                         const int &m) const {
  const int F = problem.getNbFams();

  for (int f = 0; f < F; ++f) {
    if (f == i.getFam()) {
      Job lastOccOfF = solution.getPreviousOcc(j, f, m);
      if (solution.getMaxEnd() - lastOccOfF.getStart() >
          problem.getThreshold(j))
        return 1;
    }

    else if (problem.isQualif(f, m)) {
      Job firstOccBeforeI = solution.getPreviousOcc(i, f, m);
      int nextStartOfF = solution.nextOf(i, f, m).getStart();

      if (firstOccBeforeI.getStart() != -1) {
        if (nextStartOfF != -1) {  // next = -1?
          if (nextStartOfF - firstOccBeforeI.getStart() +
                  problem.getDuration(j) >
              problem.getThreshold(f))
            return 1;
        }
      } else if (nextStartOfF != -1) {
        if (nextStartOfF + problem.getDuration(j) > problem.getThreshold(f))
          return 1;
      }
    }
  }
  return 0;
}

void QualifCentricHeuristic::updateTime(const Job &i, const Job &j,
                                        const int &k) {
  // update data for j, save j and remove j from the schedule
  Job j2(j.getFam(), i.getStart() + problem.getDuration(j.getFam()),
         j.getIndex());
  solution.removeLastJob(k);
  // shift all task after i by p_f(j)
  for (int l = 0; l < solution.getNbJobsOn(k); ++l) {
    Job cur = solution.getJobs(l, k);
    if (cur.getStart() > i.getStart()) {
      solution.shiftJob(l, k, problem.getDuration(j));
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
      // tant qu'on fait un move
      bool update;
      do {
        update = false;
        getLastGroup(deb, fin, k, nbJobs);
        int machineSelected = -1;
        Job jobSelected(-1);
        findJobMachineMatch(k, deb, fin, machineSelected, jobSelected, nbJobs);
        if (machineSelected != -1 && jobSelected.getFam() != -1) {
          update = true;
          updateTime(jobSelected, deb, nbJobs, k, machineSelected);
        }
      } while (update);
    }
  }

  int Cmax = solution.getMaxEnd();
  for (int f = 0; f < problem.getNbFams(); ++f)
    for (int i = 0; i < problem.getNbMchs(); ++i) {
      if (solution.lastOf(f, i).getStart() + problem.getThreshold(f) >= Cmax)
        solution.setDisqualif(std::numeric_limits<int>::max(), f, i);
    }
}

void QualifCentricHeuristic::findJobMachineMatch(int k, const Job &deb,
                                                 const Job &fin,
                                                 int &machineSelected,
                                                 Job &jobSelected,
                                                 int nbJobs) const {
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
              jobSelected = jobi;
              machineSelected = m;
            }
      }
    }
  }
}

int QualifCentricHeuristic::computeNewCmax(const Job &deb, const int &k,
                                           const int &m,
                                           const int &nbJobs) const {
  int cmax = 0;
  for (int i = 0; i < problem.getNbMchs(); ++i) {
    if (i == k) {
      if (cmax < solution.getEnd(k) - nbJobs * problem.getDuration(deb))
        cmax = solution.getEnd(k) - nbJobs * problem.getDuration(deb);
    } else if (i == m) {
      if (cmax < solution.getEnd(m) + nbJobs * problem.getDuration(deb))
        cmax = solution.getEnd(m) + nbJobs * problem.getDuration(deb);
    } else {
      if (cmax < solution.getEnd(i)) cmax = solution.getEnd(i);
    }
  }
  return cmax;
}

// remove [deb, deb+nbJobs] from k, add them after to jobi on m
int QualifCentricHeuristic::addDisqualif(const Job &deb, const Job &jobi,
                                         const int &m, const int &k,
                                         int nbJobs) const {
  const int F = problem.getNbFams();
  int cmax = computeNewCmax(deb, k, m, nbJobs);
  // adding jobs [deb,fin] on m add a disqualif?
  for (int f = 0; f < F; ++f) {
    if (f != jobi.getFam() && problem.isQualif(f, m)) {
      Job firstOccBeforeI = solution.getPreviousOcc(jobi, f, m);
      int nextStartOfF = solution.nextOf(jobi, f, m).getStart();

      if (firstOccBeforeI.getStart() != -1) {
        if (nextStartOfF != -1) {  // next = -1?
          if (nextStartOfF + problem.getDuration(deb) * nbJobs -
                  firstOccBeforeI.getStart() >
              problem.getThreshold(f))
            return 1;
        } else {
          if (solution.getDisqualif(f, m) == infinity &&
              cmax - firstOccBeforeI.getStart() > problem.getThreshold(f))
            return 1;
        }
      } else {
        if (nextStartOfF != -1) {
          if (nextStartOfF + problem.getDuration(deb) * nbJobs >
              problem.getThreshold(f))
            return 1;
        } else {
          if (solution.getDisqualif(f, m) == infinity &&
              cmax > problem.getThreshold(f))
            return 1;
        }
      }
    }
  }

  // removing [deb,fin] on k add a disqualif?
  Job prev = solution.getPreviousOcc(deb, deb.getFam(), k);
  if (prev.getStart() != -1) {
    if (cmax - prev.getStart() > problem.getThreshold(deb)) return 1;
  } else if (cmax > problem.getThreshold(deb))
    return 1;

  return 0;
}

int QualifCentricHeuristic::addCompletion(const Job &i, const int &nbJobs,
                                          const int &k, const int &m) const {
  return (solution.getEnd(k) <=
          solution.getEnd(m) + nbJobs * problem.getDuration(i));
}

void QualifCentricHeuristic::getLastGroup(Job &deb, Job &fin, int k,
                                          int &nbJobs) {
  fin = solution.getLastJob(k);
  int i = solution.getNbJobsOn(k) - 1;
  while (i >= 0 && solution.getJobs(i, k).getFam() == fin.getFam()) --i;
  deb = solution.getJobs(i + 1, k);
  nbJobs = solution.getNbJobsOn(k) - 1 - i;
}
// remove [deb, deb+nbJobs] from k, add them after to jobi on m
void QualifCentricHeuristic::updateTime(const Job &i, const Job &deb,
                                        int nbJobs, int k, int m) {
  int j;
  // remove nbJobs last nbJobs on k
  for (j = 0; j < nbJobs; ++j) solution.removeLastJob(k);

  // add nbJobs jobs of family f(i) after i and shift all jobs after
  // 1/ shift every job after i
  j = 0;
  while (!(solution.getJobs(j, m) == i)) j++;
  for (j = j + 1; j < solution.getNbJobsOn(m); ++j)
    solution.shiftJob(j, m, nbJobs * problem.getDuration(i));

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
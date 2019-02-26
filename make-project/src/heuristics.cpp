#include "heuristics.h"
#include <algorithm>

int ListHeuristic::chooseFamily(int m, std::vector<int> toSchedule)
{
  int selected = -1;
  for (int f = 0; f < problem.getFamilyNumber(); ++f)
  {
    if (problem.F[f].qualif[m] && toSchedule[f] != 0)
    {
      if (selected == -1)
        selected = f;
      else if (problem.F[f].threshold < problem.F[selected].threshold)
        selected = f;
      else if (problem.F[f].threshold == problem.F[selected].threshold && problem.F[f].duration < problem.F[selected].duration)
        selected = f;
    }
  }
  return selected;
}

void ListHeuristic::treat(const int m, const int f, std::vector<int> &endLast, std::vector<int> &toSchedule, std::vector<int> &nextOfFam)
{
  //remplissage de solution
  (solution.QualifLostTime[f][m] == endLast[m] ? //if the last task on j is of family f
       solution.S[nextOfFam[f]].start = endLast[m]
                                               :                          //no setup
       solution.S[nextOfFam[f]].start = endLast[m] + problem.F[f].setup); //otw setup
  solution.S[nextOfFam[f]].machine = m;
  solution.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = solution.S[nextOfFam[f]].start + problem.F[f].duration;
  solution.QualifLostTime[f][m] = endLast[m]; //update QualifLostTIme
  toSchedule[f]--;                            //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] != 0)
  {
    nextOfFam[f]++;
    while (problem.famOf[nextOfFam[f]] != f && nextOfFam[f] < problem.N)
      nextOfFam[f]++;
  }
  //disqualification?
  for (int f2 = 0; f2 < problem.getFamilyNumber(); ++f2)
  { //forall families(!= f and qualified on m)
    if (problem.F[f2].qualif[m] && f2 != f)
      if (solution.QualifLostTime[f2][m] + problem.F[f2].threshold - problem.F[f2].duration < endLast[m] + problem.F[f2].setup)
      {
        (solution.QualifLostTime[f2][m] == 0 ? solution.QualifLostTime[f2][m] += problem.F[f2].threshold : solution.QualifLostTime[f2][m] += problem.F[f2].threshold - problem.F[f2].duration);
        problem.F[f2].qualif[m] = 0;
      }
  }
}

void ListHeuristic::doSolve(ConfigAPC &config)
{
  //init and variable
  const int F = problem.getFamilyNumber();
  int i, f;
  std::vector<int> endLast(problem.M, 0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F, 0);      //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);      //next i solution.t. fam(i) == f
  for (f = 0; f < F; ++f)
    toSchedule[f] = problem.getNf(f);
  for (f = 0; f < F; ++f)
  {
    i = 0;
    while (i < problem.N && problem.famOf[i] != f)
      ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine.
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
      if (problem.F[f].qualif[i])
        solution.QualifLostTime[f][i] = 0;

  //algo
  i = 0;
  while (i < problem.N)
  {
    bool feasible = false;
    for (int j = 0; j < problem.M; ++j)
    {
      if ((f = chooseFamily(j, toSchedule)) != -1)
      {
        treat(j, f, endLast, toSchedule, nextOfFam);
        feasible = true;
        i++;
      }
    }
    if (feasible == false)
    {
      return;
    }
  }
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
      if (problem.F[f].qualif[i] || endLast[i] <= solution.QualifLostTime[f][i])
        solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
  setSAT();
}

void SchedCentricHeuristic::treat(const int m, const int f,
                                  std::vector<int> &endLast,
                                  std::vector<int> &toSchedule,
                                  std::vector<int> &nextOfFam)
{
  //remplissage de solution
  (solution.QualifLostTime[f][m] == endLast[m] ? //if the last task on j is of family f
       solution.S[nextOfFam[f]].start = endLast[m]
                                               :                          //no setup
       solution.S[nextOfFam[f]].start = endLast[m] + problem.F[f].setup); //otw setup
  solution.S[nextOfFam[f]].machine = m;
  solution.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = solution.S[nextOfFam[f]].start + problem.F[f].duration;
  solution.QualifLostTime[f][m] = endLast[m]; //update QualifLostTIme
  toSchedule[f]--;                            //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] != 0)
  {
    nextOfFam[f]++;
    while (problem.famOf[nextOfFam[f]] != f && nextOfFam[f] < problem.N)
      nextOfFam[f]++;
  }
  //disqualification?
  for (int f2 = 0; f2 < problem.getFamilyNumber(); ++f2)
  { //forall families(!= f and qualified on m)
    if (problem.F[f2].qualif[m] && f2 != f)
    {
      if (solution.QualifLostTime[f2][m] != 0)
      {
        if (solution.QualifLostTime[f2][m] + problem.F[f2].threshold - problem.F[f2].duration < endLast[m] + problem.F[f2].setup)
        {
          solution.QualifLostTime[f2][m] += problem.F[f2].threshold - problem.F[f2].duration;
          problem.F[f2].qualif[m] = 0;
        }
      }
      else
      {
        if (solution.QualifLostTime[f2][m] + problem.F[f2].threshold < endLast[m] + problem.F[f2].setup)
        {
          solution.QualifLostTime[f2][m] += problem.F[f2].threshold;
          problem.F[f2].qualif[m] = 0;
        }
      }
    }
  }
}

int SchedCentricHeuristic::remainingThresh(const int &f, const int &m, const int &t)
{
  return solution.QualifLostTime[f][m] + problem.F[f].threshold - t;
}

int SchedCentricHeuristic::chooseFamily(const int &m, const int &t, const int &current, std::vector<int> toSchedule)
{
  int critical, selected = -1;
  //if there's no task on m, choose with the minimum threshold rule
  if (current == -1)
    selected = famWithMinThresh(m, 0, toSchedule);
  else
  {
    //if there's no task of current left choose with the minimum remaining threshold rule
    if (toSchedule[current] == 0)
      return famWithMinThresh(m, t, toSchedule);
    else
    {
      selected = current;
      //select family with min remaining threshold
      if ((critical = famWithMinThresh(m, t, toSchedule)) != -1)
        if (remainingThresh(critical, m, t) - problem.F[critical].setup < problem.F[selected].duration)
          selected = critical;
    }
  }
  return selected;
}

int SchedCentricHeuristic::famWithMinThresh(const int &m, const int &t, std::vector<int> toSchedule)
{
  int selected = -1;
  for (int f = 0; f < problem.getFamilyNumber(); ++f)
  {
    if (problem.F[f].qualif[m] && toSchedule[f] != 0)
    {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m, t) < remainingThresh(selected, m, t))
        selected = f;
      else if (remainingThresh(f, m, t) == remainingThresh(selected, m, t) && problem.F[f].duration < problem.F[selected].duration)
        selected = f;
    }
  }
  return selected;
}

void SchedCentricHeuristic::doSolve(ConfigAPC &config)
{
  const int F = problem.getFamilyNumber();
  int i, f, j;
  std::vector<int> endLast(problem.M, 0); //end time of the last task scheduled of m
  std::vector<int> toSchedule(F, 0);      //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);
  std::vector<int> lastFam(problem.M, -1); //next i s.t. fam(i) == f
  for (f = 0; f < F; ++f)
    toSchedule[f] = problem.getNf(f);
  for (f = 0; f < F; ++f)
  {
    i = 0;
    while (i < problem.N && problem.famOf[i] != f)
      ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine.
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
      if (problem.F[f].qualif[i])
        solution.QualifLostTime[f][i] = 0;
  i = 0;
  while (i < problem.N)
  {
    bool feasible = false;

    for (j = 0; j < problem.M; ++j)
    {
      if ((f = chooseFamily(j, endLast[j], lastFam[j], toSchedule)) != -1)
      {
        feasible = true;
        lastFam[j] = f;
        treat(j, f, endLast, toSchedule, nextOfFam);
        ++i;
      }
    }

    if (!feasible)
      return;
  }
  int Cmax = solution.getMaxEnd(problem);
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
    {
      if (problem.F[f].qualif[i])
      {
        if (std::max(solution.QualifLostTime[f][i] - problem.F[f].duration, 0) + problem.F[f].threshold < Cmax)
          (solution.QualifLostTime[f][i] == 0 ? solution.QualifLostTime[f][i] += problem.F[f].threshold : solution.QualifLostTime[f][i] += problem.F[f].threshold - problem.F[f].duration);
        else
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
      else
      {
        if (solution.QualifLostTime[f][i] >= Cmax)
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
    }
  setSAT();
}

// std::string Heuristics::solve(ConfigAPC& config) {
//   std::vector<std::string> heuristics = config.getHeuristics();
//   for( std::string heuristic : heuristics) {
//      AbstractSolverAPC* solver = makeHeuristic(problem, heuristic);
//      std::string status = solver->solve(config);
//      std::cout << "d " << heuristic << " " << status << std::endl;
//      if( status == S_SAT) {
//         this->status == S_SAT;
//         this->solutionCount += 1;
//         // TODO Set solution if improving
//      }
//   }
//   return this->status;
// }

void QualifCentricHeuristic::doSolve(ConfigAPC &config)
{
  //FIXME REALLY NOT SURE ABOUT THIS ONE !
  std::vector<int> endLast(problem.M, 0);
  if (schedule(endLast))
  {
    //std::cout << " Phase 1 done \n La solution est valide ?" << s.isValid(P) << "\n" << s.toString(P);
    //s.toTikz(P);
    intraChange(endLast);
    //std::cout << " Phase 2 done \n  La solution est valide ?" << s.isValid(P) << "\n" << s.toString(P);
    //s.toTikz(P);
    interChange(endLast);
    setSAT();
    }
}

int QualifCentricHeuristic::treat(const int m, const int f, std::vector<int> &endLast, std::vector<int> &toSchedule, std::vector<int> &nextOfFam)
{
  //remplissage de solution
  (solution.QualifLostTime[f][m] == endLast[m] ? //if the last task on j is of family f
       solution.S[nextOfFam[f]].start = endLast[m]
                                               :                          //no setup
       solution.S[nextOfFam[f]].start = endLast[m] + problem.F[f].setup); //otw setup
  solution.S[nextOfFam[f]].machine = m;
  solution.S[nextOfFam[f]].index = nextOfFam[f];
  //update endLast
  endLast[m] = solution.S[nextOfFam[f]].start + problem.F[f].duration;
  solution.QualifLostTime[f][m] = endLast[m]; //update QualifLostTIme
  toSchedule[f]--;                            //update nf (toSchedule)
  //update nextOfFam
  if (toSchedule[f] != 0)
  {
    nextOfFam[f]++;
    while (problem.famOf[nextOfFam[f]] != f && nextOfFam[f] < problem.N)
      nextOfFam[f]++;
  }
  //disqualification?
  for (int f2 = 0; f2 < problem.getFamilyNumber(); ++f2)
  { //forall families(!= f and qualified on m)
    if (problem.F[f2].qualif[m] && f2 != f)
    {
      if (solution.QualifLostTime[f2][m] != 0)
      {
        if (solution.QualifLostTime[f2][m] + problem.F[f2].threshold - problem.F[f2].duration < endLast[m] + problem.F[f2].setup)
        {
          solution.QualifLostTime[f2][m] += problem.F[f2].threshold - problem.F[f2].duration;
          problem.F[f2].qualif[m] = 0;
        }
      }
      else
      {
        if (solution.QualifLostTime[f2][m] + problem.F[f2].threshold < endLast[m] + problem.F[f2].setup)
        {
          solution.QualifLostTime[f2][m] += problem.F[f2].threshold;
          problem.F[f2].qualif[m] = 0;
        }
      }
    }
  }
  return 0;
}

int QualifCentricHeuristic::remainingThresh(const int &f, const int &m,
                                            const int &t)
{
  return solution.QualifLostTime[f][m] + problem.F[f].threshold - t;
}

int QualifCentricHeuristic::schedule(std::vector<int> &endLast)
{
  const int F = problem.getFamilyNumber();
  int i, f, j;
  std::vector<int> toSchedule(F, 0); //number of task to schedule in f
  std::vector<int> nextOfFam(F, -1);
  for (f = 0; f < F; ++f)
    toSchedule[f] = problem.getNf(f);
  for (f = 0; f < F; ++f)
  {
    i = 0;
    while (i < problem.N && problem.famOf[i] != f)
      ++i;
    nextOfFam[f] = i;
  }
  //qualifLostTime is used to store the end time of last occurence of a family on a machine.
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
      if (problem.F[f].qualif[i])
        solution.QualifLostTime[f][i] = 0;

  i = 0;
  while (i < problem.N)
  {
    bool feasible = false;
    for (j = 0; j < problem.M; ++j)
    {
      if ((f = chooseFamily(j, endLast[j], toSchedule)) != -1)
      {
        feasible = true;
        treat(j, f, endLast, toSchedule, nextOfFam);
        ++i;
      }
    }

    if (!feasible)
      return 0;
  }

  int Cmax = solution.getMaxEnd(problem);
  for (f = 0; f < F; ++f)
    for (i = 0; i < problem.M; ++i)
    {
      if (problem.F[f].qualif[i])
      {
        if (std::max(solution.QualifLostTime[f][i] - problem.F[f].duration, 0) + problem.F[f].threshold < Cmax)
          (solution.QualifLostTime[f][i] == 0 ? solution.QualifLostTime[f][i] += problem.F[f].threshold : solution.QualifLostTime[f][i] += problem.F[f].threshold - problem.F[f].duration);
        else
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
      else
      {
        if (solution.QualifLostTime[f][i] >= Cmax)
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
    }
  return 1;
}

int QualifCentricHeuristic::chooseFamily(const int &m, const int &t,
                                         std::vector<int> toSchedule)
{
  int selected = -1;
  for (int f = 0; f < problem.getFamilyNumber(); ++f)
  {
    if (problem.F[f].qualif[m] && toSchedule[f] != 0)
    {
      if (selected == -1)
        selected = f;
      else if (remainingThresh(f, m, t) < remainingThresh(selected, m, t))
        selected = f;
      else if (remainingThresh(f, m, t) == remainingThresh(selected, m, t) && problem.F[f].duration < problem.F[selected].duration)
        selected = f;
    }
  }
  return selected;
}

int QualifCentricHeuristic::intraChange(std::vector<int> &endLast)
{
  int k, j, i = 0;
  const int F = problem.getFamilyNumber();
  for (k = 0; k < problem.M; ++k)
  { //sur chaque machine
    if (solution.getNbJobsOn(k) > 2)
    {
      std::vector<int> moved(problem.N, 0);
      std::vector<int> firstOcc(F, -1);
      i = 0;
      //compute firstOcc of each family
      while (i < problem.N)
      {
        if (solution.S[i].machine == k &&
            (firstOcc[problem.famOf[i]] == -1 ||
             solution.S[firstOcc[problem.famOf[i]]].start + problem.getDuration(firstOcc[problem.famOf[i]]) == solution.S[i].start))
          firstOcc[problem.famOf[solution.S[i].index]] = i;
        ++i;
      }
      //intraChange movement
      bool update;
      do
      {
        update = false;
        j = getLastOn(k, endLast[k]); //recupere l'index du dernier élément
        i = firstOcc[problem.famOf[solution.S[j].index]];
        if (i != -1 && i != j && !moved[solution.S[j].index] && !addDisqualif(i, j, k, k, 1))
        {
          update = true;
          moved[solution.S[j].index] = 1;
          updateTime(i, j, j, k, k, endLast);
          firstOcc[problem.famOf[solution.S[j].index]] = j;
        }
      } while (update);
    }
    int Cmax = solution.getMaxEnd(problem);
    for (int f = 0; f < problem.getFamilyNumber(); ++f)
      for (i = 0; i < problem.M; ++i)
      {
        if (solution.QualifLostTime[f][i] >= Cmax)
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }

  return 0;
}

int QualifCentricHeuristic::interChange(std::vector<int> &endLast)
{
  int k, i, j;
  std::sort(solution.S.begin(), solution.S.end());
  for (k = 0; k < problem.M; ++k)
  { // pour chaque machine
    if (solution.getNbJobsOn(k) > 1)
    {
      bool update;
      do
      { // tant qu'on fait un move
        update = false;
        // recuperer la derniere tache sur k (si il y en a une)
        if ((j = getLastOn(k, endLast[k])) != problem.N)
        {
          int firstOfLast = getBeginOfLasts(j);
          int machineSelected = -1;
          int jobSelected = -1;

          findJobMachineMatch(k, j, firstOfLast, endLast, machineSelected, jobSelected);

          if (machineSelected != -1 && jobSelected != -1)
          {
            update = true;
            updateTime(jobSelected, j, firstOfLast, k, machineSelected, endLast);
            std::sort(solution.S.begin(), solution.S.end());
          }
        }
      } while (update);
    }

    int Cmax = solution.getMaxEnd(problem);
    for (int f = 0; f < problem.getFamilyNumber(); ++f)
      for (i = 0; i < problem.M; ++i)
      {
        if (solution.QualifLostTime[f][i] >= Cmax)
          solution.QualifLostTime[f][i] = std::numeric_limits<int>::max();
      }
  }
  return 0;
}

void QualifCentricHeuristic::findJobMachineMatch(int k, int j, int firstOfLast, const std::vector<int> &endLast, int &machineSelected, int &jobSelected)
{
  for (int m = 0; m < problem.M; ++m)
  {
    if (problem.isQualif(solution.S[j].index, m) && k != m)
    { // sur quelle machine je la met...
      int i = 0;
      while (i < problem.N)
      {
        if (solution.S[i].machine == m && problem.famOf[solution.S[i].index] == problem.famOf[solution.S[j].index]) //...et avec quelle tache
          if (!addDisqualif(i, firstOfLast, m, k, j - firstOfLast + 1) && !addCompletion(solution.S[i].index, j - firstOfLast + 1, k, m, endLast))
            if (machineSelected == -1 || endLast[m] < endLast[machineSelected])
            {
              jobSelected = i;
              machineSelected = m;
            }
        ++i;
      }
    }
  }
}

int QualifCentricHeuristic::addCompletion(const int &i, const int &nbJobs, const int &k, const int &m, std::vector<int> endLast)
{
  return (endLast[k] <= endLast[m] + nbJobs * problem.getDuration(i));
}

int QualifCentricHeuristic::getLastOn(const int &k, const int &t)
{
  int i = 0;
  while (i < problem.N && (solution.S[i].machine != k || solution.S[i].start + problem.getDuration(solution.S[i].index) != t))
    ++i;
  return i;
}

int QualifCentricHeuristic::getBeginOfLasts(const int &last)
{
  int i = last;
  while (i >= 0 && problem.famOf[solution.S[i].index] == problem.famOf[solution.S[last].index])
    --i;
  return i + 1;
}

// i sur machine m ; j sur machine k
int QualifCentricHeuristic::addDisqualif(const int &i, const int &j, const int &m,
                                         const int &k, const int &nbJobs)
{
  const int F = problem.getFamilyNumber();
  std::vector<int> firstOccAfter(F, problem.N);
  std::vector<int> lastOccBef(F, -1);

  //adding j on m add a disqualif?

  //compute last occurence of each familiy before i on m
  computeLastOccBefore(m, i, lastOccBef);
  //compute first occurence of each familiy after i on m
  computeFirstOccAfter(m, i, firstOccAfter);

  //we just have two check if the "qualification" still holds between the last occ
  //of f before i and the first one after i
  for (int f = 0; f < F; ++f)
  {
    if (f != problem.famOf[solution.S[i].index] && problem.F[f].qualif[m])
    {
      if (lastOccBef[f] == -1)
      {
        if (firstOccAfter[f] < problem.N)
          if (solution.S[firstOccAfter[f]].start + nbJobs * problem.getDuration(solution.S[i].index) > problem.F[f].threshold)
            return 1;
      }
      else if (firstOccAfter[f] < problem.N)
        if (solution.S[firstOccAfter[f]].start + nbJobs * problem.getDuration(solution.S[i].index) -
                solution.S[lastOccBef[f]].start >
            problem.F[f].threshold)
          return 1;
    }
  }
  //removing j on k add a disqualif?
  lastOccBef[problem.famOf[solution.S[j].index]] = -1;
  for (int cur = 0; cur < problem.N; ++cur)
  {
    if (j != cur && problem.famOf[solution.S[j].index] == problem.famOf[solution.S[cur].index] && solution.S[cur].machine == k && solution.S[cur].start < solution.S[j].start)
    {
      if (lastOccBef[problem.famOf[solution.S[cur].index]] == -1)
        lastOccBef[problem.famOf[solution.S[cur].index]] = cur;
      else if (solution.S[lastOccBef[problem.famOf[solution.S[cur].index]]].start < solution.S[cur].start)
        lastOccBef[problem.famOf[solution.S[cur].index]] = cur;
    }
  }
  int Cmax = 0;
  if (k == m)
    Cmax = solution.getMaxEnd(problem);
  else
  {
    for (int l = 0; l < problem.M; ++l)
    {
      if (l == k)
        Cmax = std::max(Cmax, solution.getRealEnd(problem, k) - nbJobs * problem.getDuration(solution.S[i].index));
      else if (l == m)
        Cmax = std::max(Cmax, solution.getRealEnd(problem, m) + nbJobs * problem.getDuration(solution.S[i].index));
      else
        Cmax = std::max(Cmax, solution.getRealEnd(problem, l));
    }
  }
  if (lastOccBef[problem.famOf[solution.S[j].index]] == -1)
  {
    if (problem.F[problem.famOf[solution.S[j].index]].threshold < Cmax)
      return 1;
  }
  else if (solution.S[lastOccBef[problem.famOf[solution.S[j].index]]].start + problem.F[problem.famOf[solution.S[j].index]].threshold < Cmax)
    return 1;
  return 0;
}

void QualifCentricHeuristic::computeLastOccBefore(int m, int i, std::vector<int> &lastOccBef)
{
  for (int cur = 0; cur < problem.N; ++cur)
    if (solution.S[cur].machine == m && solution.S[cur].start < solution.S[i].start)
    {
      if (lastOccBef[problem.famOf[solution.S[cur].index]] == -1)
        lastOccBef[problem.famOf[solution.S[cur].index]] = cur;
      else if (solution.S[lastOccBef[problem.famOf[solution.S[cur].index]]].start < solution.S[cur].start)
        lastOccBef[problem.famOf[solution.S[cur].index]] = cur;
    }
}

void QualifCentricHeuristic::computeFirstOccAfter(int m, int i, std::vector<int> &firstOccAfter)
{
  for (int cur = 0; cur < problem.N; ++cur)
  {
    if (solution.S[cur].machine == m && solution.S[cur].start > solution.S[i].start)
    {
      if (firstOccAfter[problem.famOf[solution.S[cur].index]] == problem.N)
        firstOccAfter[problem.famOf[solution.S[cur].index]] = cur;
      else if (solution.S[firstOccAfter[problem.famOf[solution.S[cur].index]]].start > solution.S[cur].start)
        firstOccAfter[problem.famOf[solution.S[cur].index]] = cur;
    }
  }
}

int QualifCentricHeuristic::updateTime(const int &i, const int &j, const int &firstOfLast,
                                       const int &k, const int &m, std::vector<int> &endLast)
{
  int cur = 0;
  int nbJobs = j - firstOfLast + 1;
  std::vector<int> update(problem.getFamilyNumber(), 0);
  solution.S[firstOfLast].start = solution.S[i].start + problem.getDuration(solution.S[i].index);
  solution.S[firstOfLast].machine = m;
  // ???
  for (cur = firstOfLast + 1; cur <= j; ++cur)
  {
    solution.S[cur].start = solution.S[cur - 1].start + problem.getDuration(solution.S[cur - 1].index);
    solution.S[cur].machine = m;
  }

  cur = 0;
  endLast[m] += nbJobs * problem.getDuration(solution.S[j].index);
  endLast[k] = 0;
  while (cur < problem.N)
  {
    // shift all the task after j by nbJobs * p_j on m
    if (solution.S[cur].machine == m && solution.S[cur].start >= solution.S[firstOfLast].start)
    {
      if (firstOfLast > cur || cur > j)
        solution.S[cur].start += nbJobs * problem.getDuration(solution.S[j].index);
      if (solution.QualifLostTime[problem.famOf[solution.S[cur].index]][m] < std::numeric_limits<int>::max() && !update[problem.famOf[solution.S[cur].index]])
      {
        update[problem.famOf[solution.S[cur].index]] = 1;
        solution.QualifLostTime[problem.famOf[solution.S[cur].index]][m] += nbJobs * problem.getDuration(solution.S[j].index);
      }
    }
    //update endLast on k
    if (solution.S[cur].machine == k && endLast[k] < solution.S[cur].start + problem.getDuration(solution.S[cur].index))
      endLast[k] = solution.S[cur].start + problem.getDuration(solution.S[cur].index);
    ++cur;
  }
  return 0;
}

HeuristicAPC* makeHeuristic(Problem &problem, ConfigAPC &config, std::string name)
{
  if (name == H_LIST) 
    return new ListHeuristic(problem, config);
  else if (name == H_QUALIF)
    return new QualifCentricHeuristic(problem, config);
  else
    return new SchedCentricHeuristic(problem, config);
}
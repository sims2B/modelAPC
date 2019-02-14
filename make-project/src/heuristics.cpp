#include "heuristics.h"

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
  status = S_SAT;
  solutionCount = 1;
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

  status = S_SAT;
  solutionCount = 1;
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

AbstractSolverAPC* makeHeuristic(Problem& problem, std::string name)
{
  if(name == H_LIST) return new ListHeuristic(problem); 
  else return new SchedCentricHeuristic(problem);
}   
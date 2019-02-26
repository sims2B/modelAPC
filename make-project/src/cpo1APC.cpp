#include "cpo1APC.h"
#include <algorithm>

void CpoSolver1APC::doSolve(IloEnv &env)
{
  IloModel model(env);

  IloIntervalVarArray masterTask(env, problem.N + 1);
  IloIntervalVarMatrix altTasks(env, problem.M);
  IloIntervalVarMatrix disqualif(env, problem.M);
  IloIntervalSequenceVarArray mchs(env, problem.M);
  createModel(env, model, masterTask, altTasks, disqualif, mchs);
  IloCP cp(model);
  
  configure(env, cp, config);
  for (Solution sol : solutionPool)
  {
    IloSolution ilosol(env);
    solToModel(sol, masterTask, altTasks, disqualif, masterTask[problem.N], ilosol);
    cp.setStartingPoint(ilosol);
    ilosol.end();
  }

  IloBool solCPFound = cp.solve();
  solutionCount += cp.getInfo(IloCP::NumberOfSolutions);

  if (solCPFound)
  {
    modelToSol(cp, altTasks, disqualif);
  }
  setStatus(cp);
  tearDown(cp);
}

void CpoSolver1APC::modelToSol(const IloCP &cp,
                               const IloIntervalVarMatrix &altTasks,
                               const IloIntervalVarMatrix &disqualif)
{
  int i, j;
  const int m = problem.M;
  const int F = problem.getFamilyNumber();

  std::vector<int> cpt(m, 0);
  for (i = 0; i < problem.N; ++i)
    for (j = 0; j < m; ++j)
      if (problem.isQualif(i, j))
      {
        if (cp.isPresent(altTasks[j][cpt[j]]))
        {
          solution.S[i].index = i;
          solution.S[i].start = (int)cp.getStart(altTasks[j][cpt[j]]);
          solution.S[i].machine = j;
        }
        cpt[j]++;
      }

  cpt.clear();
  cpt.resize(m, 0);
  for (i = 0; i < F; ++i)
    for (j = 0; j < m; ++j)
      if (problem.F[i].qualif[j])
      {
        if (cp.isPresent(disqualif[j][cpt[j]]))
          solution.QualifLostTime[i][j] = (int)cp.getStart(disqualif[j][cpt[j]]);
        cpt[j]++;
      }
}

void CpoSolver1APC::solToModel(Solution &solution, IloIntervalVarArray &masterTask, IloIntervalVarMatrix &altTasks,
                               IloIntervalVarMatrix &disqualif, IloIntervalVar &Cmax,
                               IloSolution &sol)
{
  const int F = problem.getFamilyNumber();
  const int M = problem.M;

  Solution s2 = solution;
  s2.reaffectId(problem);
  std::sort(s2.S.begin(), s2.S.end(), idComp);

  //std::cout << s2.toString(P);
  //masterTask
  for (int i = 0; i < problem.N; ++i)
    sol.setStart(masterTask[s2.S[i].index], s2.S[i].start);
  //disqualif
  for (int j = 0; j < M; ++j)
  {
    IloInt Fcpt = 0;
    for (int f = 0; f < F; ++f)
      if (problem.F[f].qualif[j])
      {
        if (s2.QualifLostTime[f][j] < std::numeric_limits<int>::max())
          sol.setPresent(disqualif[j][Fcpt]);
        else
          sol.setAbsent(disqualif[j][Fcpt]);
        Fcpt++;
      }
  }
  //Cmax
  sol.setStart(Cmax, s2.getMaxEnd(problem));
  //altTask
  for (int j = 0; j < M; j++)
  {
    IloInt Icpt = 0;
    for (int i = 0; i < problem.N; ++i)
      if (problem.isQualif(i, j))
      {
        if (s2.S[i].machine == j)
        {
          sol.setPresent(altTasks[j][Icpt]);
          sol.setStart(altTasks[j][Icpt], s2.S[i].start);
        }
        else
          sol.setAbsent(altTasks[j][Icpt]);
        Icpt++;
      }
  }
}

void CpoSolver1APC::createModel(IloEnv &env, IloModel &model,
                                IloIntervalVarArray &masterTask, IloIntervalVarMatrix &altTasks,
                                IloIntervalVarMatrix &disqualif, IloIntervalSequenceVarArray &mchs)
{
  createVariables(env, masterTask, altTasks, disqualif, mchs);
  createConstraints(env, model, masterTask, altTasks, disqualif, mchs);
  createObjective(env, model, masterTask, disqualif);
}

void CpoSolver1APC::createVariables(IloEnv &env, IloIntervalVarArray &masterTask,
                                    IloIntervalVarMatrix &altTasks, IloIntervalVarMatrix &disqualif,
                                    IloIntervalSequenceVarArray &mchs)
{
  int i, j;
  const int n = problem.N;
  const int m = problem.M;
  const int T = problem.computeHorizon();
  char name[27];

  for (i = 0; i < n; ++i)
  {
    snprintf(name, 27, "master_%d", i);
    //  masterTask[i].setName(name);
    masterTask[i] = IloIntervalVar(env, name);
    masterTask[i].setEndMax(T);
  }
  masterTask[n] = IloIntervalVar(env, (IloInt)0);
  masterTask[n].setEndMax(T);

  for (j = 0; j < m; ++j)
  {
    altTasks[j] = IloIntervalVarArray(env);
    for (i = 0; i < n; ++i)
      if (problem.isQualif(i, j))
      {
        IloIntervalVar alt(env, problem.getDuration(i));
        snprintf(name, 27, "alt_%d_%d", j, i);
        alt.setName(name);
        alt.setOptional();
        altTasks[j].add(alt);
      }
  }

  for (j = 0; j < m; ++j)
  {
    IloIntArray types(env, altTasks[j].getSize());
    IloInt cpt = 0;
    for (i = 0; i < n; ++i)
    {
      if (problem.isQualif(i, j))
      {
        types[cpt] = problem.famOf[i];
        cpt++;
      }
    }
    mchs[j] = IloIntervalSequenceVar(env, altTasks[j], types);
  }

  for (j = 0; j < m; ++j)
  {
    disqualif[j] = IloIntervalVarArray(env);
    for (i = 0; i < problem.getFamilyNumber(); ++i)
      if (problem.F[i].qualif[j])
      {
        IloIntervalVar qual(env, (IloInt)0);
        snprintf(name, 27, "disQ_%d_%d", j, i);
        qual.setName(name);
        qual.setOptional();
        disqualif[j].add(qual);
      }
  }
}

void CpoSolver1APC::createObjective(IloEnv &env, IloModel &model,
                                    IloIntervalVarArray &masterTask, IloIntervalVarMatrix &disqualif)
{
  IloInt i;
  IloIntExprArray ends(env);

  //1 completion time
  for (i = 0; i < problem.N; ++i)
    ends.add(IloEndOf(masterTask[i]));

  //2 disqualif
  IloIntExprArray disQ(env);
  for (i = 0; i < problem.M; ++i)
  {
    IloInt Jcpt = 0;
    for (int j = 0; j < problem.getFamilyNumber(); ++j)
      if (problem.F[j].qualif[i])
      {
        disQ.add(((IloInt)1 - IloSizeOf(disqualif[i][Jcpt], (IloInt)1)));
        Jcpt++;
      }
  }

  // TODO Use config !
  if (!weighted)
  {
    IloNumExprArray objs(env);
    if (prioFlow)
    {
      objs.add(IloSum(ends));
      objs.add(IloSum(disQ));
    }
    else
    {
      objs.add(IloSum(disQ));
      objs.add(IloSum(ends));
    }

    IloMultiCriterionExpr myObj = IloStaticLex(env, objs);
    model.add(IloMinimize(env, myObj));
    objs.end();
  }
  else
    model.add(IloMinimize(env, alpha_C * IloSum(ends) + beta_Y * IloSum(disQ)));

  ends.end();
  disQ.end();
}

void CpoSolver1APC::createConstraints(IloEnv &env, IloModel &model,
                                      IloIntervalVarArray &masterTask, IloIntervalVarMatrix &altTasks,
                                      IloIntervalVarMatrix &disqualif, IloIntervalSequenceVarArray &mchs)
{
  int i, j, f;
  const int n = problem.N;
  const int m = problem.M;
  const int F = problem.getFamilyNumber();
  std::vector<int> cptMach(m, 0);

  // un seul "mode" est choisie pour une tâche
  for (i = 0; i < n; ++i)
  {
    IloIntervalVarArray members(env);
    for (j = 0; j < m; ++j)
      if (problem.isQualif(i, j))
      {
        members.add(altTasks[j][cptMach[j]]);
        cptMach[j]++;
      }
    model.add(IloAlternative(env, masterTask[i], members));
  }

  //setup
  IloTransitionDistance setup(env, F);
  for (i = 0; i < F; ++i)
    for (j = 0; j < F; ++j)
      (i != j ? setup.setValue(i, j, problem.F[j].setup) : setup.setValue(i, j, 0));
  for (j = 0; j < m; j++)
    model.add(IloNoOverlap(env, mchs[j], setup, IloTrue));

  // threshold ( disqualif last executed of family on a machine)
  for (j = 0; j < m; j++)
  {
    IloInt Fcpt = 0;
    for (f = 0; f < F; ++f)
      if (problem.F[f].qualif[j])
      {
        IloInt Icpt = 0;
        for (i = 0; i < n; ++i)
          if (problem.famOf[i] == f)
          {
            model.add(IloStartBeforeStart(env, altTasks[j][Icpt],
                                          disqualif[j][Fcpt], problem.F[f].threshold));
            Icpt++;
          }
          else if (problem.isQualif(i, j))
            Icpt++;
        Fcpt++;
      }
  }

  // threshold (if a task of f is executed on j, then an other one of f has to be
  // executed before gamma_f OR the machine j becomes disqualified for f OR no other
  // task is scheduled on j (end_j)
  for (j = 0; j < m; j++)
  {
    IloInt Icpt = 0;
    for (i = 0; i < n; ++i)
    {
      if (problem.isQualif(i, j))
      {
        IloOr c(env);
        IloInt I2cpt = Icpt + 1;
        for (int i2 = i + 1; i2 < n; ++i2)
        {
          if (problem.isQualif(i2, j))
          {
            if (problem.famOf[i] == problem.famOf[i2])
              c = c || (IloPresenceOf(env, altTasks[j][I2cpt]) && IloStartOf(altTasks[j][I2cpt]) <=
                                                                      IloStartOf(altTasks[j][Icpt]) + problem.getThreshold(i));
            I2cpt++;
          }
        }
        IloInt Fcpt = 0;
        for (f = 0; f < problem.famOf[i]; ++f)
          if (problem.F[f].qualif[j])
            Fcpt++;
        c = c || IloStartOf(altTasks[j][Icpt]) + problem.getThreshold(i) ==
                     IloStartOf(disqualif[j][Fcpt]);
        c = c || IloStartOf(altTasks[j][Icpt]) + problem.getThreshold(i) >=
                     IloStartOf(masterTask[n]);
        model.add(IloIfThen(env, IloPresenceOf(env, altTasks[j][Icpt]), c));
        Icpt++;
      }
    }
  }

  // the machine becomes disqualified for f OR end_j <= gamma_f
  // if there is  no task of family f executed on a qualified machine j,
  for (j = 0; j < m; j++)
  {
    IloInt Fcpt = 0;
    for (f = 0; f < F; ++f)
      if (problem.F[f].qualif[j])
      {
        IloInt Icpt = 0;
        IloOr c(env);
        for (i = 0; i < n; ++i)
        {
          if (problem.famOf[i] == f)
          {
            c = c || (IloPresenceOf(env, altTasks[j][Icpt]) && IloStartOf(altTasks[j][Icpt]) <= problem.F[f].threshold);
            Icpt++;
          }
          else if (problem.isQualif(i, j))
            Icpt++;
        }
        c = c || (IloPresenceOf(env, disqualif[j][Fcpt]) && problem.F[f].threshold ==
                                                                IloStartOf(disqualif[j][Fcpt]));
        c = c || IloStartOf(masterTask[n]) <= problem.F[f].threshold;
        model.add(c);
        Fcpt++;
      }
  }

  // end_j is the last task on j
  for (j = 0; j < m; j++)
  {
    IloInt Icpt = 0;
    for (i = 0; i < n; ++i)
      if (problem.isQualif(i, j))
      {
        model.add(IloEndBeforeStart(env, altTasks[j][Icpt],
                                    masterTask[n]));
        Icpt++;
      }
  }

  // ordonne les start des taches d'une même famille (optionnelle)
  // (version globale)

  for (i = 0; i < n; ++i)
  {
    int j = i + 1;
    while (j < n && problem.famOf[i] != problem.famOf[j])
      ++j;
    if (j < n)
      model.add(IloStartBeforeStart(env, masterTask[i], masterTask[j]));
  }
}
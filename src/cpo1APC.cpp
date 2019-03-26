#include "cpo1APC.h"
#include <algorithm>

void CpoSolver1APC::doSolve(IloEnv &env) {
  const int N = problem.getNbJobs();
  const int M = problem.getNbMchs();
  IloModel model(env);

  IloIntervalVarArray masterTask(env, N + 1);
  IloIntervalVarMatrix altTasks(env, M);
  IloIntervalVarMatrix disqualif(env, M);
  IloIntervalSequenceVarArray mchs(env, M);
  createModel(env, model, masterTask, altTasks, disqualif, mchs);
  IloCP cp(model);
  configure(env, cp, config);
  for (Solution sol : solutionPool) {
    IloSolution ilosol(env);
    solToModel(sol, masterTask, altTasks, disqualif, masterTask[N], ilosol);
    cp.setStartingPoint(ilosol);
    ilosol.end();
  }

  IloBool solCPFound = iloSolve(cp);
  solutionCount += cp.getInfo(IloCP::NumberOfSolutions);
  if (solCPFound) {
    modelToSol(cp, altTasks, disqualif);
  }
  setStatus(cp);
  tearDown(cp);
}

void CpoSolver1APC::modelToSol(const IloCP &cp,
                               const IloIntervalVarMatrix &altTasks,
                               const IloIntervalVarMatrix &disqualif) {
  int i, j, f;
  const int m = problem.getNbMchs();
  const int F = problem.getNbFams();

  for (j = 0; j < m; ++j) {
    int deb, fin = 0;
    for (f = 0; f < problem.getNbFams(); ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        for (i = deb; i < fin; ++i) {
          if (cp.isPresent(altTasks[j][i])) {
            solution.addJob(Job(f, (int)cp.getStart(altTasks[j][i]), -1), j);
          }
        }
      }
    }
  }

  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.isQualif(f, j))
        if (cp.isPresent(disqualif[j][f]))
          solution.setDisqualif((int)cp.getStart(disqualif[j][f]), f, j);
}

void CpoSolver1APC::solToModel(const Solution &solution,
                               IloIntervalVarArray &masterTask,
                               IloIntervalVarMatrix &altTasks,
                               IloIntervalVarMatrix &disqualif,
                               IloIntervalVar &Cmax, IloSolution &sol) {
  const int F = problem.getNbFams();
  const int M = problem.getNbMchs();

  std::vector<Job> solList;
  for (int m = 0; m < problem.getNbMchs(); ++m)
    for (int j = 0; j < solution.getNbJobsOn(m); ++j)
      solList.push_back(solution.getJobs(j, m));
  std::sort(solList.begin(),
            solList.end());  // tri par famille (f0,f0,f0,....,f1,f1,f1,...)

  // masterTask
  for (int i = 0; i < problem.getNbJobs(); ++i)
    sol.setStart(masterTask[i], solList[i].getStart());

  // disqualif
  int cmax = solution.getMaxEnd();
  for (int j = 0; j < M; ++j) {
    for (int f = 0; f < F; ++f)
      if (problem.isQualif(f, j) && solution.getDisqualif(f, j) < cmax)
        sol.setPresent(disqualif[j][f]);
      else
        sol.setAbsent(disqualif[j][f]);
  }
  // Cmax
  sol.setStart(masterTask[problem.getNbJobs()], cmax);

  // altTask
  Solution copie(solution);
  int deb, fin = 0;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    deb = fin;
    fin = deb + problem.getNbJobs(f);
    for (IloInt j = deb; j < fin; ++j) {
      Job a(f);
      int mch;
      copie.getFirstOcc(a, f, mch);
      copie.removeJob(a, mch);
      for (IloInt k = 0; k < problem.getNbMchs(); ++k) {
        if (mch == k) {
          sol.setPresent(altTasks[k][j]);
          sol.setStart(altTasks[k][j], a.getStart());
        } else
          sol.setAbsent(altTasks[k][j]);
      }
    }
  }
}

void CpoSolver1APC::createModel(IloEnv &env, IloModel &model,
                                IloIntervalVarArray &masterTask,
                                IloIntervalVarMatrix &altTasks,
                                IloIntervalVarMatrix &disqualif,
                                IloIntervalSequenceVarArray &mchs) {
  createVariables(env, masterTask, altTasks, disqualif, mchs);
  createConstraints(env, model, masterTask, altTasks, disqualif, mchs);
  createObjective(env, model, masterTask, disqualif);
}

void CpoSolver1APC::createVariables(IloEnv &env,
                                    IloIntervalVarArray &masterTask,
                                    IloIntervalVarMatrix &altTasks,
                                    IloIntervalVarMatrix &disqualif,
                                    IloIntervalSequenceVarArray &mchs) {
  int i, j;
  const int n = problem.getNbJobs();
  const int m = problem.getNbMchs();
  const int T = problem.computeHorizon();
  char name[27];

  for (i = 0; i < n; ++i) {
    snprintf(name, 27, "master_%d", i);
    //  masterTask[i].setName(name);
    masterTask[i] = IloIntervalVar(env, name);
    masterTask[i].setEndMax(T);
  }
  masterTask[n] = IloIntervalVar(env, (IloInt)0);
  masterTask[i].setName("cmax");
  masterTask[n].setEndMax(T);

  for (j = 0; j < m; ++j) {
    altTasks[j] = IloIntervalVarArray(env);
    int deb, fin = 0;
    for (int f = 0; f < problem.getNbFams(); ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      for (i = deb; i < fin; ++i) {
        IloIntervalVar alt(env, problem.getDuration(f));
        snprintf(name, 27, "alt_%d_%d", j, i);
        alt.setName(name);
        alt.setOptional();
        altTasks[j].add(alt);
      }
    }
  }

  for (j = 0; j < m; ++j) {
    IloIntArray types(env, altTasks[j].getSize());
    int deb, fin = 0;
    for (int f = 0; f < problem.getNbFams(); ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      for (i = deb; i < fin; ++i) {
        types[i] = f;
      }
    }
    mchs[j] = IloIntervalSequenceVar(env, altTasks[j], types);
  }

  for (j = 0; j < m; ++j) {
    disqualif[j] = IloIntervalVarArray(env);
    for (i = 0; i < problem.getNbFams(); ++i) {
      IloIntervalVar qual(env, (IloInt)0);
      snprintf(name, 27, "disQ_%d_%d", j, i);
      qual.setName(name);
      qual.setOptional();
      disqualif[j].add(qual);
    }
  }
}

void CpoSolver1APC::createObjective(IloEnv &env, IloModel &model,
                                    IloIntervalVarArray &masterTask,
                                    IloIntervalVarMatrix &disqualif) {
  IloInt i;
  IloIntExprArray ends(env);

  // 1 completion time
  for (i = 0; i < problem.getNbJobs(); ++i) ends.add(IloEndOf(masterTask[i]));

  // 2 disqualif
  IloIntExprArray disQ(env);
  for (int j = 0; j < problem.getNbMchs(); ++j) {
    for (i = 0; i < problem.getNbFams(); ++i)
      if (problem.isQualif(i, j))
        disQ.add(((IloInt)1 - IloSizeOf(disqualif[j][i], (IloInt)1)));
  }

  if (config.getObjectiveFunction() == "MONO") {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, IloSum(ends)));
    else
      model.add(IloMinimize(env, IloSum(disQ)));
  }

  else if (config.getObjectiveFunction() == "LEX") {
    IloNumExprArray objs(env);
    if (config.getWeightFlowtime() > config.getWeightQualified()) {
      objs.add(IloSum(ends));
      objs.add(IloSum(disQ));
    } else {
      objs.add(IloSum(disQ));
      objs.add(IloSum(ends));
    }
    IloMultiCriterionExpr myObj = IloStaticLex(env, objs);
    model.add(IloMinimize(env, myObj));
    objs.end();
  }

  else {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, IloSum(ends) + IloSum(disQ)));
    else {
      double beta = problem.getNbJobs() * problem.computeHorizon();
      model.add(IloMinimize(env, IloSum(ends) + beta * IloSum(disQ)));
    }
  }
  // TODO Use config !

  ends.end();
  disQ.end();
}

void CpoSolver1APC::createConstraints(IloEnv &env, IloModel &model,
                                      IloIntervalVarArray &masterTask,
                                      IloIntervalVarMatrix &altTasks,
                                      IloIntervalVarMatrix &disqualif,
                                      IloIntervalSequenceVarArray &mchs) {
  int i, j, f, deb, fin;
  const int n = problem.getNbJobs();
  const int m = problem.getNbMchs();
  const int F = problem.getNbFams();

  // un seul "mode" est choisie pour une tâche
  for (i = 0; i < n; ++i) {
    IloIntervalVarArray members(env);
    for (j = 0; j < m; ++j) members.add(altTasks[j][i]);
    model.add(IloAlternative(env, masterTask[i], members));
  }

  // unqualified (f(i),j) => altTasks' are absent

  for (j = 0; j < m; ++j) {
    deb = 0;
    fin = 0;
    for (f = 0; f < F; ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (!problem.isQualif(f, j))
        for (i = deb; i < fin; ++i)
          model.add(!IloPresenceOf(env, altTasks[j][i]));
    }
  }
  // unqualified (f(i),j) => disqualif' are absent
  for (j = 0; j < m; ++j) {
    for (f = 0; f < F; ++f)
      if (!problem.isQualif(f, j))
        model.add(!IloPresenceOf(env, disqualif[j][f]));
  }

  // setup
  IloTransitionDistance setup(env, F);
  for (i = 0; i < F; ++i)
    for (j = 0; j < F; ++j)
      (i != j ? setup.setValue(i, j, problem.getSetup(j))
              : setup.setValue(i, j, 0));
  for (j = 0; j < m; j++) model.add(IloNoOverlap(env, mchs[j], setup, IloTrue));

  // threshold ( disqualif last executed of family on a machine)
  for (j = 0; j < m; j++) {
    deb = 0;
    fin = 0;
    for (f = 0; f < F; ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        for (i = deb; i < fin; ++i) {
          model.add(IloStartBeforeStart(env, altTasks[j][i], disqualif[j][f],
                                        problem.getThreshold(f)));
        }
      }
    }
  }

  // threshold (if a task of f is executed on j, then an other one of f
  // has to be executed before gamma_f OR the machine j becomes
  // disqualified for f OR no other task is scheduled on j (end_j)
  for (j = 0; j < m; j++) {
    deb = 0;
    fin = 0;
    for (f = 0; f < F; ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        for (i = deb; i < fin; ++i) {
          IloOr c(env);
          for (int i2 = i + 1; i2 < fin; ++i2) {
            c = c || (IloPresenceOf(env, altTasks[j][i2]) &&
                      IloStartOf(altTasks[j][i2]) <=
                          IloStartOf(altTasks[j][i]) + problem.getThreshold(f));
          }
          c = c || IloStartOf(altTasks[j][i]) + problem.getThreshold(f) ==
                       IloStartOf(disqualif[j][f]);

          c = c || IloStartOf(altTasks[j][i]) + problem.getThreshold(f) >=
                       IloStartOf(masterTask[n]);

          model.add(IloIfThen(env, IloPresenceOf(env, altTasks[j][i]), c));
        }
      }
    }
  }
  // the machine becomes disqualified for f OR end_j <= gamma_f
  // if there is  no task of family f executed on a qualified machine
  // j,
  for (j = 0; j < m; j++) {
    deb = 0;
    fin = 0;
    for (f = 0; f < F; ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        IloOr c(env);
        for (i = deb; i < fin; ++i) {
          c = c || (IloPresenceOf(env, altTasks[j][i]) &&
                    IloStartOf(altTasks[j][i]) <= problem.getThreshold(f));
        }
        c = c || (IloPresenceOf(env, disqualif[j][f]) &&
                  problem.getThreshold(f) == IloStartOf(disqualif[j][f]));
        c = c || IloStartOf(masterTask[n]) <= problem.getThreshold(f);
        model.add(c);
      }
    }
  }

  // end_j is the last task on j
  for (j = 0; j < m; j++) {
    deb = 0;
    fin = 0;
    for (f = 0; f < F; ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        for (i = deb; i < fin; ++i) {
          model.add(IloEndBeforeStart(env, altTasks[j][i], masterTask[n]));
        }
      }
    }
  }

  // ordonne les getStart des taches d'une même famille (optionnelle)
  // (version globale)
  deb = 0;
  fin = 0;
  for (f = 0; f < F; ++f) {
    deb = fin;
    fin = deb + problem.getNbJobs(f);
    for (i = deb; i < fin; ++i) {
      int j = i + 1;
      while (j < fin) {
        model.add(IloStartBeforeStart(env, masterTask[i], masterTask[j]));
        j++;
      }
    }
  }
}

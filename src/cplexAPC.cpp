#include "cplexAPC.h"

void CplexSolverAPC::doSolve(IloEnv &env) {
  const int F = problem.getNbFams();
  const int T = problem.computeHorizon();
  const int N = problem.getNbJobs();
  IloModel model(env);

  IloNumVar3DMatrix x(env, F);
  IloNumVar3DMatrix y(env, F);
  IloNumVarArray C(env, F, 0, N * (T + 1), ILOFLOAT);
  IloNumVarMatrix Y(env, F);

  createModel(T, env, model, x, y, C, Y);
  IloCplex cplex(model);
  configure(env, cplex, config);
  for (Solution sol : solutionPool) {
    IloNumVarArray startVar(env);
    IloNumArray startVal(env);
    solToModel(sol, x, y, Y, C, startVar, startVal);
    cplex.addMIPStart(startVar, startVal);
    startVar.end();
    startVal.end();
  }
  IloBool solMIPFound = iloSolve(cplex);
  solutionCount += cplex.getSolnPoolNsolns();
  if (solMIPFound) {
    modelToSol(cplex, x, y, Y);
    if (!checkObjValue(cplex,C,Y)) std::cerr << "WARNING : objective value between CPLEX and solution does not match" << std::endl;
  }
  setStatus(cplex);
  tearDown(cplex);
}

int CplexSolverAPC::checkObjValue(const IloCplex &cplex,
                                const IloNumVarArray &C,
                                const IloNumVarMatrix &Y){
  
  if (!(config.getObjectiveFunction() == "MONO" && config.getWeightFlowtime() < config.getWeightQualified())){
    int flowTime = 0;
  for (int f = 0 ; f < problem.getNbFams() ; ++f){
    flowTime += cplex.getValue(C[f]);
  }
  if (flowTime != solution.getSumCompletion()) return 0;
                                }
  if (!(config.getObjectiveFunction() == "MONO" && config.getWeightFlowtime() > config.getWeightQualified())){
  int nbDisqualif = 0;
  int nbQualif = 0;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    for (int j = 0; j < problem.getNbMchs(); ++j)
      if (problem.isQualif(f, j)) {
        nbQualif  += 1 - cplex.getValue(Y[f][j]);
        nbDisqualif += cplex.getValue(Y[f][j]);
      }
  }
  if (nbQualif != solution.getNbQualif () || nbDisqualif != solution.getNbDisqualif() )
     return 0;
  }
  return 1;
}

void CplexSolverAPC::modelToSol(const IloCplex &cplex,
                                const IloNumVar3DMatrix &x,
                                const IloNumVar3DMatrix &y,
                                const IloNumVarMatrix &Y) {
  const int F = problem.getNbFams();
  const int T = problem.computeHorizon();
  int f, m, t;
  for (f = 0; f < F; ++f)
    for (m = 0; m < problem.getNbMchs(); ++m)
      if (problem.isQualif(f, m))
        for (t = 0; t < T - problem.getDuration(f); ++t) {
          if (IloRound(cplex.getValue(x[f][m][t])) == 1)
            solution.addJob(Job(f, t, -1), m);
        }

  for (f = 0; f < F; ++f)
    for (m = 0; m < problem.getNbMchs(); ++m)
      if (problem.isQualif(f, m) && IloRound(cplex.getValue(Y[f][m])))
        for (t = 0; t < T; ++t)
          if (IloRound(cplex.getValue(y[f][m][t])) == 1) {
            solution.setDisqualif(t, f, m);
            break;
          }
  solution.repairDisqualif();  // utile?
}

void CplexSolverAPC::solToModel(Solution &solution, IloNumVar3DMatrix &x,
                                IloNumVar3DMatrix &y, IloNumVarMatrix &Y,
                                IloNumVarArray &C, IloNumVarArray &startVar,
                                IloNumArray &startVal) {
  const int T = problem.computeHorizon();
  const int F = problem.getNbFams();
  const int M = problem.getNbMchs();
  int i, m, f, t;
  // remplissage de x

  for (m = 0; m < M; ++m)
    for (i = 0; i < solution.getNbJobsOn(m); ++i) {
      startVar.add(x[solution.getJobs(i, m).getFam()][m]
                    [solution.getJobs(i, m).getStart()]);
      startVal.add(1);
    }

  // remplissage de y
  for (f = 0; f < F; ++f)
    for (m = 0; m < M; ++m)
      if (problem.isQualif(f, m)) {
        if (solution.getDisqualif(f, m) < std::numeric_limits<int>::max()) {
          t = 0;
          while (t < T) {
            startVar.add(y[f][m][t]);
            if (t < solution.getDisqualif(f, m))
              startVal.add(0);
            else
              startVal.add(1);
            ++t;
          }
        }
      }

  // remplissage de Y
  for (f = 0; f < F; ++f)
    for (m = 0; m < M; ++m) {
      if (problem.isQualif(f, m)) {
        startVar.add(Y[f][m]);
        if (solution.getDisqualif(f, m) == std::numeric_limits<int>::max())
          startVal.add(0);
        else
          startVal.add(1);
      }
    }

  // remplissage de C
  for (f = 0; f < problem.getNbFams(); ++f) {
    int sum = 0;
    for (m = 0; m < M; ++m)
      for (i = 0; i < solution.getNbJobsOn(m); ++i)
        if (solution.getJobs(i, m).getFam() == f)
          sum += solution.getJobs(i, m).getStart() + problem.getDuration(f);
    startVar.add(C[f]);
    startVal.add(sum);
  }
}

void CplexSolverAPC::createModel(int T, IloEnv &env, IloModel &model,
                                 IloNumVar3DMatrix &x, IloNumVar3DMatrix &y,
                                 IloNumVarArray &C, IloNumVarMatrix &Y) {
  createVars(T, env, x, y, Y);
  createObj(env, model, C, Y);
  createConstraints(T, env, model, x, y, C, Y);
}

void CplexSolverAPC::createVars(int T, IloEnv &env, IloNumVar3DMatrix &x,
                                IloNumVar3DMatrix &y, IloNumVarMatrix &Y) {
  int i, j;
  const int M = problem.getNbMchs();
  IloNumArray Y_ub(env, M);

  char namevar[24];
  for (i = 0; i < problem.getNbFams(); ++i) {
    x[i] = IloNumVarMatrix(env, M);
    for (j = 0; j < M; ++j) {
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
      for (int t = 0; t < T; ++t) {
        snprintf(namevar, 24, "x_%d_%d_%d", i, t, j);
        x[i][j][t].setName(namevar);
      }
    }
  }
  for (i = 0; i < problem.getNbFams(); ++i) {
    y[i] = IloNumVarMatrix(env, M);
    for (j = 0; j < M; ++j) {
      if (problem.isQualif(i, j)) {
        y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
        Y_ub[j] = 1;
      } else {
        y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT);
        Y_ub[j] = 0;
      }
      for (int t = 0; t < T; ++t) {
        snprintf(namevar, 24, "y_%d_%d_%d", i, t, j);
        y[i][j][t].setName(namevar);
      }
    }

    Y[i] = IloNumVarArray(env, 0, Y_ub, ILOINT);
    for (int j = 0; j < M; ++j) {
      snprintf(namevar, 24, "Y_%d_%d", i, j);
      Y[i][j].setName(namevar);
    }
  }
}

void CplexSolverAPC::createObj(IloEnv &env, IloModel &model, IloNumVarArray &C,
                               IloNumVarMatrix &Y) {
  assert(config.getObjectiveFunction() != "LEX" &&
         "Lexical objective not compatible with MIP");

  IloExpr expr1(env);
  IloExpr expr2(env);
  for (int f = 0; f < problem.getNbFams(); ++f) {
    expr1 += C[f];
    for (int j = 0; j < problem.getNbMchs(); ++j)
      if (problem.isQualif(f, j)) expr2 += Y[f][j];
  }

  if (config.getObjectiveFunction() == "MONO") {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, expr1));
    else
      model.add(IloMinimize(env, expr2));
  } else {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, expr1 + expr2));
    else {
      double beta = problem.getNbJobs() * problem.computeHorizon();
      model.add(IloMinimize(env, expr1 + beta * expr2));
    }
  }

  expr1.end();
  expr2.end();
}

void CplexSolverAPC::createConstraints(int T, IloEnv &env, IloModel &model,
                                       IloNumVar3DMatrix &x,
                                       IloNumVar3DMatrix &y, IloNumVarArray &C,
                                       IloNumVarMatrix &Y) {
  int j, t, f, tau;

  const int M = problem.getNbMchs();
  const int F = problem.getNbFams();

  // each job is scheduled once
  for (f = 0; f < F; ++f) {
    IloExpr expr(env);
    for (j = 0; j < M; ++j) {
      if (problem.isQualif(f, j)) {
        for (t = 0; t <= T - problem.getDuration(f); ++t) expr += x[f][j][t];
      }
    }
    model.add(expr == problem.getNbJobs(f));
    expr.end();
  }

  // completion time of a job
  for (f = 0; f < F; ++f) {
    IloExpr expr(env);
    for (j = 0; j < M; ++j) {
      if (problem.isQualif(f, j)) {
        for (t = 0; t <= T - problem.getDuration(f); ++t)
          expr += ((t + problem.getDuration(f)) * x[f][j][t]);
      }
    }
    model.add(expr <= C[f]);
    expr.end();
  }

  // noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
    for (t = problem.getDuration(f) - 1; t <= T - problem.getDuration(f); ++t)
      for (j = 0; j < M; ++j)
        if (problem.isQualif(f, j)) {
          IloExpr expr(env);
          for (tau = t - problem.getDuration(f) + 1; tau <= t; ++tau)
            expr += x[f][j][tau];
          expr += y[f][j][t];
          model.add(expr <= 1);
          expr.end();
        }

  // noOverlap on the same machine with setup time
  for (f = 0; f < F; ++f)
    for (int f2 = 0; f2 < F; ++f2) {
      if (f != f2) {
        for (t = 0 /*problem.getDuration(f) + problem.F[f2].setup - 1*/;
             t <= T - problem.getDuration(f2); ++t)
          for (j = 0; j < M; ++j)
            if (!((problem.isQualif(f, j) + problem.isQualif(f2, j)) % 2)) {
              IloExpr expr(env);
              for (tau = std::max(0, t - problem.getDuration(f) -
                                         problem.getSetup(f2) + 1);
                   tau <= t; ++tau)
                expr += x[f][j][tau];
              expr += problem.getNbJobs(f) * x[f2][j][t];
              model.add(expr <= problem.getNbJobs(f));
              expr.end();
            }
      }
    }

  // threshold
  for (f = 0; f < F; ++f)
    for (j = 0; j < M; ++j)
      if (problem.isQualif(f, j)) {
        for (t = problem.getThreshold(f); t < T; ++t) {
          IloExpr expr(env);
          for (tau = t - problem.getThreshold(f) + 1; tau <= t; ++tau)
            expr += x[f][j][tau];
          expr += y[f][j][t];
          model.add(expr >= 1);
          expr.end();
        }
      }

  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < M; ++j)
        if (problem.isQualif(f, j)) {
          IloExpr expr(env);
          for (int f2 = 0; f2 < F; ++f2) {
            for (tau = std::max(0, t - problem.getDuration(f2)); tau < T;
                 ++tau) {
              for (int j2 = 0; j2 < M; ++j2) {
                if (problem.isQualif(f2, j)) {
                  expr += x[f2][j2][tau];
                }
              }
            }
          }
          expr = expr / (T - t);
          expr += y[f][j][t - 1] - 1;
          model.add(expr <= Y[f][j]);
          expr.end();
        }

  // if a machine become disqualified, it stays disqualified
  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < M; ++j)
        if (problem.isQualif(f, j)) model.add(y[f][j][t - 1] <= y[f][j][t]);
}

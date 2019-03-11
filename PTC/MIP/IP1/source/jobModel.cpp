#include "jobModel.h"

int solve(const Problem& P, Solution& s){
  try{
    IloNum getStart;
    const int n = problem.N;
    const int T = problem.computeHorizon();
    IloEnv env;
    IloModel model(env);

    IloNumVar3DMatrix x(env, n);
    IloNumVar3DMatrix y(env, problem.getNbFams());
    IloNumVarArray C(env, n, 0, T + 1, ILOFLOAT);

    if (!createModel(P, T, env, model, x, y, C)){
      IloCplex cplex(model);

      setParam(env, cplex);

      getStart = cplex.getCplexTime();

      //solve!
      if (cplex.solve() || cplex.getStatus() == IloAlgorithm::Infeasible){
	if (cplex.getStatus() == IloAlgorithm::Infeasible){

	  int ret = displayCVS(P, s, cplex, getStart);
	  //|| displayCplexSolution(P,env,cplex,x,y,C);
	  env.end();
	  return ret;
	}
	else {
	  int ret =//displayCplexSolution(P,env,cplex,x,y,C)||
	    modelToSol(P, s, cplex, x, y) ||
	    displayCVS(P, s, cplex, getStart);
	  env.end();
	  return ret;
	}
      }
    }
    env.end();
    return 1;
  }
  catch (IloException &e){
    std::cout << "Iloexception in solve" << e << std::endl;
    e.end();
    return 1;
  }
  catch (...){
    std::cout << "Error unknown\n";
    return 1;
  }
}



int displayCplexSolution(const Problem& P, IloEnv& env, IloCplex& cplex, const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y, const IloNumVarArray& C){
  IloNumArray v(env);

  // affichage de x et C
  for (int i = 0; i < problem.N; ++i) {
    for (int j = 0; j < problem.M; ++j)
      if (problem.isQualif(i, j)){
	IloInt t = 0;
	while (t < x[i][j].getSize() - problem.getDuration(i) &&
	       IloRound(cplex.getValue(x[i][j][t])) != 1)
	  ++t;
	if (t < x[i][j].getSize() - problem.getDuration(i)){
	  std::cout << "x[" << i << "][" << j << "]=";
	  for (IloInt tau = 0; tau < x[i][j].getSize() - problem.getDuration(i); ++tau)
	    std::cout << cplex.getValue(x[i][j][tau]) << " , ";
	}
      }
    std::cout << "\n C[" << i << "]=" << cplex.getValue(C[i]) << std::endl;
  }

  for (int f = 0; f < problem.getNbFams(); ++f)
    for (int j = 0; j < problem.M; ++j)
      if (problem.F[f].qualif[j]){
	cplex.getValues(y[f][j], v);
	std::cout << "y[" << f << "][" << j << "]=" << v << std::endl;
      }

  return 0;
}

int displayCplexResults(const IloCplex& cplex, const IloNum& getStart){
  IloNum time_exec = cplex.getCplexTime() - getStart;
  std::cout << "Final status: \t" << cplex.getStatus() << " en "
	    << time_exec << std::endl;
  if (!(cplex.getStatus() == IloAlgorithm::Infeasible)){
    std::cout << "Final objective: " << cplex.getObjValue()
	      << "\nFinal gap: \t" << cplex.getMIPRelativeGap()
	      << std::endl;
  }
  return 0;
}


int displayCVS(const Problem& P, const Solution& s, const IloCplex& cplex, const IloNum& getStart){
  IloNum time_exec = cplex.getCplexTime() - getStart;
  std::cout << time_exec << ";";
  if (!(cplex.getStatus() == IloAlgorithm::Infeasible)){
    std::cout << "1;1;";
    if (cplex.getStatus() == IloAlgorithm::Optimal)
      std::cout << "1;";
    else std::cout << "0;";
    std::cout << IloRound(cplex.getObjValue()) << ";" << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P) << ";" << cplex.getMIPRelativeGap();
  }
  else
    std::cout << "1; ; ; ; ; ; ; ; ";
  return 0;
}

int setParam(IloEnv& env, IloCplex& cplex){
  cplex.setParam(IloCplex::TiLim, time_limit);
  cplex.setParam(IloCplex::Threads, 2);
  cplex.setOut(env.getNullStream());
  return 0;
}

int modelToSol(const Problem& P, Solution& s, IloCplex& cplex, IloNumVar3DMatrix& x,
	       IloNumVar3DMatrix& y){

  const int F = problem.getNbFams();
  const int T = problem.computeHorizon();
  int i, j, t, f;

  for (i = 0; i < problem.N; ++i)
    for (j = 0; j < problem.M; ++j)
      if (problem.isQualif(i, j))
	for (t = 0; t < x[i][j].getSize() - problem.getDuration(i); ++t)
	  if (IloRound(cplex.getValue(x[i][j][t])) == 1)
	    s.assign[i] = Assignment(t, j, i);

  for (f = 0; f < F; ++f)
    for (j = 0; j < problem.M; ++j)
      if (problem.F[f].qualif[j]){
	t = 0;
	while (t < T && IloRound(cplex.getValue(y[f][j][t])) != 1)
	  ++t;
	if (t < T)
	  s.qualifLostTime[f][j] = t;
      }
  return 0;
}

int createModel(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		IloNumVar3DMatrix& y, IloNumVarArray& C){
  return createVars(P, T, env, x, y) || createConstraints(P, T, env, model, x, y, C);
}

int createVars(const Problem& P, int T, IloEnv& env, IloNumVar3DMatrix& x,
	       IloNumVar3DMatrix& y){
  IloInt i, j;


  for (i = 0; i < problem.N; ++i){
    x[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j)
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
  }
  for (i = 0; i < problem.getNbFams(); ++i){
    y[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j){
      (problem.F[i].qualif[j] ? y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT) :
       y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT));
    }
  }
  return 0;
}

int createConstraints(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		      IloNumVar3DMatrix& y, IloNumVarArray& C){
  int i, j, t, f, tau;
  const int m = problem.M;
  const int n = problem.N;
  const int F = problem.getNbFams();

  //objective
  IloExpr expr(env);
  for (i = 0; i < n; ++i)
    expr += alpha_C * C[i];
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j])
	expr += beta_Y * y[f][j][T - 1];
  model.add(IloMinimize(env, expr));
  expr.end();

  //each job is scheduled once
  for (i = 0; i < n; ++i){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      
      if (problem.isQualif(i, j)){
	for (t = 0; t < T - problem.getDuration(i); ++t)
	  expr += x[i][j][t];
      }
    }
    model.add(expr == 1);
    expr.end();
  }

  //completion time of a job
  for (i = 0; i < n; ++i){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (problem.isQualif(i, j)){
	for (t = 0; t < T - problem.getDuration(i); ++t)
	  expr += (t * x[i][j][t]);
      }
    }
    expr += problem.getDuration(i);
    model.add(expr <= C[i]);
    expr.end();
  }

  //noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
    for (t = problem.getDuration(f); t < T; ++t)
      for (j = 0; j < m; ++j)
	if (problem.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (problem.famOf[i] == f)
	      for (tau = t - problem.getDuration(i); tau < t; ++tau)
		expr += x[i][j][tau];
	  model.add(expr <= 1);
	  expr.end();
	}


  //noOverlap on the same machine with setup time
  for (i = 0; i < n; ++i)
    for (int i2 = i; i2 < n; ++i2){
      for (t = 0; t < T; ++t)
	for (j = 0; j < m; ++j)
	  if (problem.famOf[i] != problem.famOf[i2] &&
	      !((problem.isQualif(i, j) + problem.isQualif(i2, j)) % 2)){
	    //  std::cout << "("<<i << "," <<i2<<") : "<< j<< "," << t << std::endl; 
	    IloExpr expr(env);
	    for (tau = std::max(0, (int)t - problem.getDuration(i) - problem.getSetup(i2));
		 tau < t; ++tau)
	      expr += x[i][j][tau];
	    for (tau = std::max(0, (int)t - problem.getDuration(i2) - problem.getSetup(i));
		 tau >= 0 && tau < t; ++tau)
	      expr += x[i2][j][tau];
	    model.add(expr <= 1);
	    expr.end();
	  }
    }

  //threshold 
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j]){
	for (t = 0; t < std::min(T, problem.getThreshold(f)); ++t){
	  model.add(y[f][j][t] == 0);
	}
	for (t = problem.getThreshold(f); t < T; ++t){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (problem.famOf[i] == f)
	      for (tau = std::max(0, (int)t - problem.getThreshold(f) + 1); tau < t; ++tau)
		expr += x[i][j][tau];
	  //expr= expr/problem.getThreshold(f);
	  expr += y[f][j][t];
	  model.add(expr > 0);
	  expr.end();
	}
      }

  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j]){
	for (t = problem.getThreshold(f); t < T; ++t){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (problem.famOf[i] == f)
	      for (tau = std::max(0, (int)t - problem.getThreshold(f) + 1); tau < t; ++tau)
		expr += x[i][j][tau];
	  expr = expr / problem.getThreshold(f);
	  expr += y[f][j][t];
	  model.add(expr <= 1);
	  expr.end();
	}
      }


  //if a machine become disqualified, it stays disqualified
  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < m; ++j)
	if (problem.F[f].qualif[j])
	  model.add(y[f][j][t - 1] <= y[f][j][t]);


  return 0;
}

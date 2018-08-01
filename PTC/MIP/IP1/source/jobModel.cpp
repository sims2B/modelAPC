#include "jobModel.h"

int solve(const Problem& P, Solution& s){
  try{
    IloNum start;
    const int n = P.N;
    const int T = P.computeHorizon();
    IloEnv env;
    IloModel model(env);

    IloNumVar3DMatrix x(env, n);
    IloNumVar3DMatrix y(env, P.getFamilyNumber());
    IloNumVarArray C(env, n, 0, T + 1, ILOFLOAT);

    if (!createModel(P, T, env, model, x, y, C)){
      IloCplex cplex(model);

      setParam(env, cplex);

      start = cplex.getCplexTime();

      //solve!
      if (cplex.solve() || cplex.getStatus() == IloAlgorithm::Infeasible){
	if (cplex.getStatus() == IloAlgorithm::Infeasible){

	  int ret = displayCVS(P, s, cplex, start);
	  //|| displayCplexSolution(P,env,cplex,x,y,C);
	  env.end();
	  return ret;
	}
	else {
	  int ret =//displayCplexSolution(P,env,cplex,x,y,C)||
	    modelToSol(P, s, cplex, x, y) ||
	    displayCVS(P, s, cplex, start);
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
  for (int i = 0; i < P.N; ++i) {
    for (int j = 0; j < P.M; ++j)
      if (P.isQualif(i, j)){
	IloInt t = 0;
	while (t < x[i][j].getSize() - P.getDuration(i) &&
	       IloRound(cplex.getValue(x[i][j][t])) != 1)
	  ++t;
	if (t < x[i][j].getSize() - P.getDuration(i)){
	  std::cout << "x[" << i << "][" << j << "]=";
	  for (IloInt tau = 0; tau < x[i][j].getSize() - P.getDuration(i); ++tau)
	    std::cout << cplex.getValue(x[i][j][tau]) << " , ";
	}
      }
    std::cout << "\n C[" << i << "]=" << cplex.getValue(C[i]) << std::endl;
  }

  for (int f = 0; f < P.getFamilyNumber(); ++f)
    for (int j = 0; j < P.M; ++j)
      if (P.F[f].qualif[j]){
	cplex.getValues(y[f][j], v);
	std::cout << "y[" << f << "][" << j << "]=" << v << std::endl;
      }

  return 0;
}

int displayCplexResults(const IloCplex& cplex, const IloNum& start){
  IloNum time_exec = cplex.getCplexTime() - start;
  std::cout << "Final status: \t" << cplex.getStatus() << " en "
	    << time_exec << std::endl;
  if (!(cplex.getStatus() == IloAlgorithm::Infeasible)){
    std::cout << "Final objective: " << cplex.getObjValue()
	      << "\nFinal gap: \t" << cplex.getMIPRelativeGap()
	      << std::endl;
  }
  return 0;
}


int displayCVS(const Problem& P, const Solution& s, const IloCplex& cplex, const IloNum& start){
  IloNum time_exec = cplex.getCplexTime() - start;
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

  const int F = P.getFamilyNumber();
  const int T = P.computeHorizon();
  int i, j, t, f;

  for (i = 0; i < P.N; ++i)
    for (j = 0; j < P.M; ++j)
      if (P.isQualif(i, j))
	for (t = 0; t < x[i][j].getSize() - P.getDuration(i); ++t)
	  if (IloRound(cplex.getValue(x[i][j][t])) == 1)
	    s.S[i] = Assignment(t, j, i);

  for (f = 0; f < F; ++f)
    for (j = 0; j < P.M; ++j)
      if (P.F[f].qualif[j]){
	t = 0;
	while (t < T && IloRound(cplex.getValue(y[f][j][t])) != 1)
	  ++t;
	if (t < T)
	  s.QualifLostTime[f][j] = t;
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


  for (i = 0; i < P.N; ++i){
    x[i] = IloNumVarMatrix(env, P.M);
    for (j = 0; j < P.M; ++j)
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
  }
  for (i = 0; i < P.getFamilyNumber(); ++i){
    y[i] = IloNumVarMatrix(env, P.M);
    for (j = 0; j < P.M; ++j){
      (P.F[i].qualif[j] ? y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT) :
       y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT));
    }
  }
  return 0;
}

int createConstraints(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		      IloNumVar3DMatrix& y, IloNumVarArray& C){
  int i, j, t, f, tau;
  const int m = P.M;
  const int n = P.N;
  const int F = P.getFamilyNumber();

  //objective
  IloExpr expr(env);
  for (i = 0; i < n; ++i)
    expr += alpha * C[i];
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (P.F[f].qualif[j])
	expr += beta * y[f][j][T - 1];
  model.add(IloMinimize(env, expr));
  expr.end();

  //each job is scheduled once
  for (i = 0; i < n; ++i){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (P.isQualif(i, j)){
	for (t = 0; t < T - P.getDuration(i); ++t)
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
      if (P.isQualif(i, j)){
	for (t = 0; t < T - P.getDuration(i); ++t)
	  expr += (t * x[i][j][t]);
      }
    }
    expr += P.getDuration(i);
    model.add(expr <= C[i]);
    expr.end();
  }

  //noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
    for (t = P.F[f].duration; t < T; ++t)
      for (j = 0; j < m; ++j)
	if (P.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (P.famOf[i] == f)
	      for (tau = t - P.getDuration(i); tau < t; ++tau)
		expr += x[i][j][tau];
	  model.add(expr <= 1);
	  expr.end();
	}


  //noOverlap on the same machine with setup time
  for (i = 0; i < n; ++i)
    for (int i2 = i; i2 < n; ++i2){
      for (t = 0; t < T; ++t)
	for (j = 0; j < m; ++j)
	  if (P.famOf[i] != P.famOf[i2] &&
	      !((P.isQualif(i, j) + P.isQualif(i2, j)) % 2)){
	    //  std::cout << "("<<i << "," <<i2<<") : "<< j<< "," << t << std::endl; 
	    IloExpr expr(env);
	    for (tau = std::max(0, (int)t - P.getDuration(i) - P.getSetup(i2));
		 tau < t; ++tau)
	      expr += x[i][j][tau];
	    for (tau = std::max(0, (int)t - P.getDuration(i2) - P.getSetup(i));
		 tau >= 0 && tau < t; ++tau)
	      expr += x[i2][j][tau];
	    model.add(expr <= 1);
	    expr.end();
	  }
    }

  //threshold 
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (P.F[f].qualif[j]){
	for (t = 0; t < std::min(T, P.F[f].threshold); ++t){
	  model.add(y[f][j][t] == 0);
	}
	for (t = P.F[f].threshold; t < T; ++t){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (P.famOf[i] == f)
	      for (tau = std::max(0, (int)t - P.F[f].threshold + 1); tau < t; ++tau)
		expr += x[i][j][tau];
	  //expr= expr/P.F[f].threshold;
	  expr += y[f][j][t];
	  model.add(expr > 0);
	  expr.end();
	}
      }

  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (P.F[f].qualif[j]){
	for (t = P.F[f].threshold; t < T; ++t){
	  IloExpr expr(env);
	  for (i = 0; i < n; ++i)
	    if (P.famOf[i] == f)
	      for (tau = std::max(0, (int)t - P.F[f].threshold + 1); tau < t; ++tau)
		expr += x[i][j][tau];
	  expr = expr / P.F[f].threshold;
	  expr += y[f][j][t];
	  model.add(expr <= 1);
	  expr.end();
	}
      }


  //if a machine become disqualified, it stays disqualified
  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < m; ++j)
	if (P.F[f].qualif[j])
	  model.add(y[f][j][t - 1] <= y[f][j][t]);


  return 0;
}

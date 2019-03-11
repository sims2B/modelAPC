#include "familyModel.h"
int solve(const Problem& P, Solution& s){
  try{
    IloNum getStart;
    const int F = problem.getNbFams();
    const int T = problem.computeHorizon();
    IloEnv env;
    IloModel model(env);

    IloNumVar3DMatrix x(env, F);
    IloNumVar3DMatrix y(env, F);
    IloNumVarArray C(env, F, 0, problem.N*(T + 1), ILOFLOAT);

    if (!createModel(P, T, env, model, x, y, C)){
      IloCplex cplex(model);

      setParam(env, cplex);

      getStart = cplex.getCplexTime();
      //cplex.exportModel("model.lp");
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

  int f, j;
  const int F = problem.getNbFams();
  // affichage de x et C
  for (f = 0; f < F; ++f) {
    for (j = 0; j < problem.M; ++j)
      if (problem.F[f].qualif[j]){
	IloInt t = 0;
	while (t < x[f][j].getSize() - problem.getDuration(f) &&
	       IloRound(cplex.getValue(x[f][j][t])) != 1)
	  ++t;
	if (t < x[f][j].getSize() - problem.getDuration(f)){
	  std::cout << "x[" << f << "][" << j << "]=";
	  for (IloInt tau = 0; tau < x[f][j].getSize() - problem.getDuration(f); ++tau)
	    std::cout << cplex.getValue(x[f][j][tau]) << " , ";
	  std::cout << std::endl;
	}
      }
    std::cout << "\n C[" << f << "]=" << cplex.getValue(C[f]) << std::endl;
  }

  for (f = 0; f < F; ++f)
    for (j = 0; j < problem.M; ++j)
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
    std::cout << cplex.getObjValue() << ";" << s.getSumCompletion(P) << ";"
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
  std::vector<int> found(F, 0);

  for (int i = 0; i < problem.N; ++i){
    int cpt = 0;
    int j = 0;
    int trouve = 0;
    while (j < problem.M && !trouve){
      if (problem.isQualif(i, j)){
	int t = 0;
	while (t < T - problem.getDuration(i) && !trouve){
	  if (IloRound(cplex.getValue(x[problem.famOf[i]][j][t])) == 1){
	    ++cpt;
	    if (cpt == found[problem.famOf[i]] + 1){
	      s.assign[i] = Assignment(t, j, i);
	      found[problem.famOf[i]]++;
	      trouve = 1;
	    }
	  }
	  ++t;
	}
      }
      ++j;
    }
  }

  for (int f = 0; f < F; ++f)
    for (int j = 0; j < problem.M; ++j)
      if (problem.F[f].qualif[j])
	for (int t = 0; t < T; ++t)
	  if (IloRound(cplex.getValue(y[f][j][t])) == 1){
	    s.qualifLostTime[f][j] = t;
	    break;
	  }
  return 0;
}

int createModel(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		IloNumVar3DMatrix& y, IloNumVarArray& C){
  return createVars(P, T, env, x, y) || createConstraints(P, T, env, model, x, y, C);
}

int createVars(const Problem& P, int T, IloEnv& env, IloNumVar3DMatrix& x,
	       IloNumVar3DMatrix& y){
  int i, j;

  char namevar[24];
  for (i = 0; i < problem.getNbFams(); ++i){
    x[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j){
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
      for (int t = 0; t < T; ++t){
	snprintf(namevar,24, "x_%d_%d_%d", i, t, j);
	x[i][j][t].setName(namevar);
      }
    }
  }
  for (i = 0; i < problem.getNbFams(); ++i){
    y[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j){
      (problem.F[i].qualif[j] ? y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT) :
       y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT));
      for (int t = 0; t < T; ++t){
	snprintf(namevar,24, "y_%d_%d_%d", i, t, j);
	y[i][j][t].setName(namevar);
      }
    }
  }

  return 0;
}

int createConstraints(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		      IloNumVar3DMatrix& y, IloNumVarArray& C){



  int j, t, f, tau;
  const int m = problem.M;
  const int F = problem.getNbFams();
  std::vector<int> nf(F);
  for (int q = 0; q < F; ++q)
    nf[q] = problem.getNf(q);


  //objective
  IloExpr expr_obj(env);
  for (f = 0; f < F; ++f)
    expr_obj += alpha_C * C[f];
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j])
	expr_obj += beta_Y * y[f][j][T - 1];
  model.add(IloMinimize(env, expr_obj));
  expr_obj.end();

  //each job is scheduled once
  for (f = 0; f < F; ++f){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (problem.F[f].qualif[j]){
	for (t = 0; t < T - problem.getDuration(f); ++t)
	  expr += x[f][j][t];
      }
    }
    model.add(expr == nf[f]);
    expr.end();
  }

  //completion time of a job
  for (f = 0; f < F; ++f){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (problem.F[f].qualif[j]){
	for (t = 0; t < T - problem.getDuration(f); ++t)
	  expr += ((t + problem.getDuration(f)) * x[f][j][t]);
      }
    }
    model.add(expr <= C[f]);
    expr.end();
  }




  //noOverlap on the same machine with setup time
  for (f = 0; f < F; ++f)
    for (int f2 = 0; f2 < F; ++f2){
      if (f != f2){
	for (t = 1; t < T; ++t)
	  for (j = 0; j < m; ++j)
	    if (!((problem.F[f].qualif[j] + problem.F[f2].qualif[j]) % 2)){
	      IloExpr expr(env);
	      for (tau = std::max(0, (int)t - problem.getDuration(f) - problem.F[f2].setup);
		   tau < t; ++tau)
		expr += x[f][j][tau];
	      expr += nf[f] * x[f2][j][t-1];
	      model.add(expr <= nf[f]);
	      expr.end();
	    }
      }
    }

  //noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
	  for (t = problem.getDuration(f); t < T; ++t)
		  for (j = 0; j < m; ++j)
			  if (problem.F[f].qualif[j]){
				  IloExpr expr(env);
				  for (tau = t - problem.getDuration(f); tau < t; ++tau)
					  expr += x[f][j][tau];
				  expr += y[f][j][t - 1];
				  model.add(expr <= 1);
				  expr.end();
			  }


  //threshold 
  for (f = 0; f < F; ++f)
	  for (j = 0; j < m; ++j)
		  if (problem.F[f].qualif[j]){
			  for (t = problem.getThreshold(f); t < T; ++t){
				  IloExpr expr(env);
				  for (tau = t - problem.getThreshold(f); tau < t; ++tau)
					  expr += x[f][j][tau];
				  expr += y[f][j][t];
				  model.add(expr >= 1);
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

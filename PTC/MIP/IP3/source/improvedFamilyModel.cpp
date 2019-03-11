
#include "improvedFamilyModel.h"
#include "paramModelAPC.h"
#include "utils.h"

int solve(const Problem& P, Solution& s){ 
  Clock::time_point startRunTime = Clock::now();
  try{
    IloNum startWC;
    const int F = problem.getNbFams();
    const int T = problem.computeHorizon();
    IloEnv env;
    IloModel model(env);

    IloNumVar3DMatrix x(env, F);
    IloNumVar3DMatrix y(env, F);
    IloNumVarArray C(env, F, 0, problem.N*(T + 1), ILOFLOAT);
    IloNumVarMatrix Y(env, F);

    if (!createModel(P, T, env, model, x, y, C, Y)){
      IloCplex cplex(model);
      if (!VERBOSITY)   cplex.setOut(env.getNullStream());
      setParam(env, cplex);

      
      Solution solSCH(P);
      Solution solQCH(P);
      if (withMIPStart){
	useMIPStart(P, solSCH, solQCH, env, cplex, x, y, Y, C);
      }
      startWC = cplex.getCplexTime();
      //solve!
      IloBool solMIPFound = cplex.solve();
      if (solMIPFound)
	modelToSol(P, s, cplex, x, y, Y);
      displayCPAIOR(P, s, solSCH, solQCH,cplex, startRunTime, startWC, solMIPFound);
    } 
    env.end();
    return 0;
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

  int useMIPStart(const Problem &P, Solution& solSCH, Solution& solQCH,IloEnv& env, IloCplex& cplex,
		IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C){
    int nbHSol = 0;
  if (SCH(P, solSCH)){
    IloNumVarArray startVar(env);
    IloNumArray startVal(env);
    solToModel(P,solSCH,x,y,Y,C,startVar,startVal);
    cplex.addMIPStart(startVar, startVal);
    startVar.end();
    startVal.end();
    nbHSol++;
  }
  else solSCH.clear(P);
  if (QCH(P, solQCH)){
    IloNumVarArray startVar(env);
    IloNumArray startVal(env);
    solToModel(P, solQCH, x,y,Y,C,startVar,startVal);
    cplex.addMIPStart(startVar, startVal);
    startVar.end();
    startVal.end();
    nbHSol++;
  }
  else solQCH.clear(P);
  std::cout << "d NBHSOL " << nbHSol << std::endl;
  return 0;
}

int displayCVS(const Problem& P, const Solution& s, const IloCplex& cplex, const IloNum& getStart){
  IloNum time_exec = cplex.getCplexTime() - getStart;
  std::cout << time_exec << ";1;";
  if (!(cplex.getStatus() == IloAlgorithm::Infeasible)){
    std::cout << cplex.getObjValue() << ";" << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P) << ";" << s.isValid(P) << ";1;";
    if (cplex.getStatus() == IloAlgorithm::Optimal)
      std::cout << "1\n";
    else std::cout << "0\n";
  }
  else
    std::cout << " ; ; ; ; ; ; \n";
  return 0;
}

int displayCPAIOR(const Problem& P, const Solution& s,const Solution& solSCH, const Solution& solQCH, const IloCplex& cplex,  Clock::time_point t1, IloNum getStart, IloBool& solved){
  Clock::time_point t2 = Clock::now();
  std::cout << "s "  << cplex.getStatus() << std::endl;
  
  std::cout << "d WCTIME " <<cplex.getCplexTime() - getStart<< "\n";

  std::chrono::duration<double> duration =
    std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
  std::cout << "d RUNTIME "<< duration.count() << "\n";
  if (solved){
    std::cout << "d CMAX " << s.getMaxEnd(P) << "\n";
  std::cout << "d FLOWTIME " << s.getSumCompletion(P) << "\n";
  std::cout << "d DISQUALIFIED "<< s.getRealNbDisqualif(P) << "\n";
  std::cout << "d QUALIFIED "<< s.getNbQualif(P) << "\n";
  std::cout << "d SETUP "<< s.getNbSetup(P) << "\n";
  std::cout << "d VALIDE "<< s.isValid(P) << "\n";
  std::cout << "d GAP " << cplex.getMIPRelativeGap() << "\n";
  int objSolMIP = s.getWeigthedObjectiveValue(P);
  if (objSolMIP == solSCH.getWeigthedObjectiveValue(P) ||
      objSolMIP == solQCH.getWeigthedObjectiveValue(P)  )
    std::cout << "d NBSOLS 0 \n";
  else
    std::cout << "d NBSOLS "  <<  cplex.getSolnPoolNsolns() << "\n";
  }
  else std::cout << "d NBSOLS 0 \n";
  std::cout << "d NBNODES " << cplex.getNnodes() << "\n";
  std::cout << "c VARIABLES " <<  cplex.getNcols() << "\n";
  std::cout << "c CONSTRAINTS " <<  cplex.getNrows() << "\n";
  std::cout << "c MACHINES "<< problem.M << "\n";
  std::cout << "c FAMILIES "<< problem.getNbFams() << "\n";
  std::cout << "c JOBS "<<problem.N << "\n";

  std::cout << std::endl;
  if (solved) s.toTikz(P);
 return 0;
}

int displayCplexSolution(const Problem& P, IloEnv& env, IloCplex& cplex,
			 const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y,
			 const IloNumVarArray& C, const IloNumVarMatrix& Y){
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

  for (f = 0; f < F; ++f){
    for (j = 0; j < problem.M; ++j)
      if (problem.F[f].qualif[j]){
	cplex.getValues(y[f][j], v);
	std::cout << "y[" << f << "][" << j << "]=" << v << std::endl;
	std::cout << "Y[" << f << "]=" << cplex.getValue(Y[f][j]) << std::endl;
      }
    //std::cout << "Y[" << f << "]=";
    //cplex.getValues(Y[f], v);
    //std::cout << v << std::endl;
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

int setParam(IloEnv& env, IloCplex& cplex){
  cplex.setParam(IloCplex::TiLim, time_limit);
  cplex.setParam(IloCplex::Threads, 2);
    return 0;
}

int modelToSol(const Problem& P, Solution& s, const IloCplex& cplex,
	       const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y,
	       const IloNumVarMatrix& Y){

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
      if (problem.F[f].qualif[j] && IloRound(cplex.getValue(Y[f][j])))
	for (int t = 0; t < T; ++t)
	  if (IloRound(cplex.getValue(y[f][j][t])) == 1){
	    s.qualifLostTime[f][j] = t;
	    break;
	  }
  s.repairDisqualif(P);
  return 0;
}

int solToModel(const Problem& P, const Solution& s, IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C, IloNumVarArray& startVar, IloNumArray& startVal){
  const int T = problem.computeHorizon();
  const int F = problem.getNbFams();
  const int M = problem.M;

  //remplissage de x
  for (int i = 0; i < problem.N; ++i){
    startVar.add(x[problem.famOf[s.assign[i].getIndex()]][s.assign[i].getMachine][s.assign[i].getStart()]);
    startVal.add(1);
  }

  //remplissage de y
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < M; ++j)
      if (problem.F[f].qualif[j]){
	if (s.qualifLostTime[f][j] < std::numeric_limits<int>::max()){
	  int t = 0;
	  while (t < T){
	    startVar.add(y[f][j][t]);
	    if (t < s.qualifLostTime[f][j]) startVal.add(0);
	    else startVal.add(1);
	    ++t;
	  }
	}
      }

  //remplissage de Y
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < M; ++j){
      if (problem.F[f].qualif[j]){
	startVar.add(Y[f][j]);
	if (s.qualifLostTime[f][j] == std::numeric_limits<int>::max())
	  startVal.add(0);
	else startVal.add(1);
      }
    }

  //remplissage de C
  for (int f = 0; f < problem.getNbFams(); ++f){
    int sum = 0;
    for (int i = 0; i < problem.N; ++i)
      (problem.famOf[s.assign[i].getIndex()] == f ? sum += s.assign[i].getStart() + problem.getDuration(f) : sum = sum);
    startVar.add(C[f]);
    startVal.add(sum);
  }
  return 0;
}

int createModel(const Problem& P, int T, IloEnv& env, IloModel& model,
		IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarArray& C,
		IloNumVarMatrix& Y){
  return createVars(P, T, env, x, y, Y) || createConstraints(P, T, env, model, x, y, C, Y);
}

int createVars(const Problem& P, int T, IloEnv& env, IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y){ 
  int i, j;

  IloNumArray Y_ub(env, problem.M);

  char namevar[24];
  for (i = 0; i < problem.getNbFams(); ++i){
    x[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j){
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
      for (int t = 0; t < T; ++t){
	snprintf(namevar, 24 , "x_%d_%d_%d", i, t, j);
	x[i][j][t].setName(namevar);
      }
    }
  }
  for (i = 0; i < problem.getNbFams(); ++i){
    y[i] = IloNumVarMatrix(env, problem.M);
    for (j = 0; j < problem.M; ++j){
      if (problem.F[i].qualif[j]){
	y[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
	Y_ub[j] = 1;
      }
      else {
	y[i][j] = IloNumVarArray(env, T, 0, 0, ILOINT);
	Y_ub[j] = 0;
      }
      for (int t = 0; t < T; ++t){
	snprintf(namevar, 24, "y_%d_%d_%d", i, t, j);
	y[i][j][t].setName(namevar);
      }
    }

    Y[i] = IloNumVarArray(env, 0, Y_ub, ILOINT);
    for (int j = 0; j < problem.M; ++j){
      snprintf(namevar, 24, "Y_%d_%d", i, j);
      Y[i][j].setName(namevar);
    }
  }
  return 0;
}

int createConstraints(const Problem& P, int T, IloEnv& env, IloModel& model,
		      IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarArray& C,
		      IloNumVarMatrix& Y){
  int j, t, f, tau;
  const int m = problem.M;
  const int F = problem.getNbFams();
  std::vector<int> nf(F);
  for (int q = 0; q < F; ++q)
    nf[q] = problem.getNf(q);


  //objective
  IloExpr expr(env);
  for (f = 0; f < F; ++f)
    expr += alpha_C * C[f];
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j])
	expr += beta_Y * Y[f][j];
  model.add(IloMinimize(env, expr));
  expr.end();

  //each job is scheduled once
  for (f = 0; f < F; ++f){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (problem.F[f].qualif[j]){
	for (t = 0; t <= T - problem.getDuration(f); ++t)
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
	for (t = 0; t <= T - problem.getDuration(f); ++t)
	  expr += ((t + problem.getDuration(f)) * x[f][j][t]);
      }
    }
    model.add(expr <= C[f]);
    expr.end();
  }

  //noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
    for (t = problem.getDuration(f) - 1; t <= T - problem.getDuration(f); ++t)
      for (j = 0; j < m; ++j)
	if (problem.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (tau = t - problem.getDuration(f) + 1; tau <= t; ++tau)
	    expr += x[f][j][tau];
	  expr += y[f][j][t];
	  model.add(expr <= 1);
	  expr.end();
	}


  //noOverlap on the same machine with setup time
  for (f = 0; f < F; ++f)
    for (int f2 = 0; f2 < F; ++f2){
      if (f != f2){
	for (t = 0/*problem.getDuration(f) + problem.F[f2].setup - 1*/; t <= T - problem.F[f2].duration; ++t)
	  for (j = 0; j < m; ++j)
	    if (!((problem.F[f].qualif[j] + problem.F[f2].qualif[j]) % 2)){
	      IloExpr expr(env);
	      for (tau = std::max(0,t - problem.getDuration(f) - problem.F[f2].setup + 1);
		   tau <= t; ++tau)
		expr += x[f][j][tau];
	      expr += nf[f] * x[f2][j][t];
	      model.add(expr <= nf[f]);
	      expr.end();
	    }
      }
    }

  //threshold 
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (problem.F[f].qualif[j]){
	for (t = problem.getThreshold(f) ; t < T; ++t){
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
      for (j = 0; j < m; ++j)
	if (problem.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (int f2 = 0; f2 < F; ++f2){
	    for (tau = std::max(0 , t - problem.F[f2].duration); tau < T; ++tau){
	      for (int j2 = 0; j2 < m; ++j2){
		if (problem.F[f2].qualif[j2]){
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

  //if a machine become disqualified, it stays disqualified
  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < m; ++j)
	if (problem.F[f].qualif[j])
	  model.add(y[f][j][t - 1] <= y[f][j][t]);


  return 0;
}

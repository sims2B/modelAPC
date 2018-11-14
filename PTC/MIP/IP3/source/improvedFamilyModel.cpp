
#include "improvedFamilyModel.h"
#include "paramModelAPC.h"
#include "utils.h"

int solve(const Problem& P, Solution& s){ 
  Clock::time_point startRunTime = Clock::now();
  try{
    IloNum startWC;
    const int F = P.getFamilyNumber();
    const int T = P.computeHorizon();
    IloEnv env;
    IloModel model(env);

    IloNumVar3DMatrix x(env, F);
    IloNumVar3DMatrix y(env, F);
    IloNumVarArray C(env, F, 0, P.N*(T + 1), ILOFLOAT);
    IloNumVarMatrix Y(env, F);

    if (!createModel(P, T, env, model, x, y, C, Y)){
      IloCplex cplex(model);
      if (!VERBOSITY)   cplex.setOut(env.getNullStream());
      setParam(env, cplex);
      if (withMIPStart)
	useMIPStart(P, env, cplex, x, y, Y, C);

      startWC = cplex.getCplexTime();

      //solve!
      if (cplex.solve()){
	modelToSol(P, s, cplex, x, y, Y);
	displayCPAIOR(P, s, cplex, startRunTime, startWC, 1);
      }
      else displayCPAIOR(P, s, cplex,  startRunTime, startWC, 0);
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

int useMIPStart(const Problem &P, IloEnv& env, IloCplex& cplex,
		IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C){
  Solution solSCH(P);
  if (SCH(P, solSCH)){
    IloNumVarArray startVar(env);
    IloNumArray startVal(env);
    solToModel(P,solSCH,x,y,Y,C,startVar,startVal);
    cplex.addMIPStart(startVar, startVal);
    startVar.end();
    startVal.end();
  }
    Solution solQCH(P);
  if (QCH(P, solQCH)){
    IloNumVarArray startVar(env);
    IloNumArray startVal(env);
    solToModel(P,solQCH,x,y,Y,C,startVar,startVal);
    cplex.addMIPStart(startVar, startVal);
    startVar.end();
    startVal.end();
  }
  return 0;
}
int displayCVS(const Problem& P, const Solution& s, const IloCplex& cplex, const IloNum& start){
  IloNum time_exec = cplex.getCplexTime() - start;
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

int displayCPAIOR(const Problem& P, const Solution& s, const IloCplex& cplex,  Clock::time_point t1, IloNum start, int solved){
  Clock::time_point t2 = Clock::now();
  
  std::cout << "s "  << cplex.getStatus() << std::endl;
  
  std::cout << "d WCTIME " <<cplex.getCplexTime() - start<< "\n";

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
  }
  std::cout << "d NBSOLS "  <<  cplex.getSolnPoolNsolns() << "\n";
  std::cout << "d GAP " << cplex.getMIPRelativeGap() << "\n";
  std::cout << "d NBNODES " << cplex.getNnodes() << "\n";
  std::cout << "c VARIABLES " <<  cplex.getNcols() << "\n";
  std::cout << "c CONSTRAINTS " <<  cplex.getNrows() << "\n";
  std::cout << "c MACHINES "<< P.M << "\n";
  std::cout << "c FAMILIES "<< P.getFamilyNumber() << "\n";
  std::cout << "c JOBS "<<P.N << "\n";

  std::cout << std::endl;
  if (solved) s.toTikz(P);
 return 0;
}

int displayCplexSolution(const Problem& P, IloEnv& env, IloCplex& cplex,
			 const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y,
			 const IloNumVarArray& C, const IloNumVarMatrix& Y){
  IloNumArray v(env);

  int f, j;
  const int F = P.getFamilyNumber();
  // affichage de x et C
  for (f = 0; f < F; ++f) {
    for (j = 0; j < P.M; ++j)
      if (P.F[f].qualif[j]){
	IloInt t = 0;
	while (t < x[f][j].getSize() - P.F[f].duration &&
	       IloRound(cplex.getValue(x[f][j][t])) != 1)
	  ++t;
	if (t < x[f][j].getSize() - P.F[f].duration){
	  std::cout << "x[" << f << "][" << j << "]=";
	  for (IloInt tau = 0; tau < x[f][j].getSize() - P.F[f].duration; ++tau)
	    std::cout << cplex.getValue(x[f][j][tau]) << " , ";
	  std::cout << std::endl;
	}
      }
    std::cout << "\n C[" << f << "]=" << cplex.getValue(C[f]) << std::endl;
  }

  for (f = 0; f < F; ++f){
    for (j = 0; j < P.M; ++j)
      if (P.F[f].qualif[j]){
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

int setParam(IloEnv& env, IloCplex& cplex){
  cplex.setParam(IloCplex::TiLim, time_limit);
  cplex.setParam(IloCplex::Threads, 2);
    return 0;
}

int modelToSol(const Problem& P, Solution& s, const IloCplex& cplex,
	       const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y,
	       const IloNumVarMatrix& Y){

  const int F = P.getFamilyNumber();
  const int T = P.computeHorizon();
  std::vector<int> found(F, 0);

  for (int i = 0; i < P.N; ++i){
    int cpt = 0;
    int j = 0;
    int trouve = 0;
    while (j < P.M && !trouve){
      if (P.isQualif(i, j)){
	int t = 0;
	while (t < T - P.getDuration(i) && !trouve){
	  if (IloRound(cplex.getValue(x[P.famOf[i]][j][t])) == 1){
	    ++cpt;
	    if (cpt == found[P.famOf[i]] + 1){
	      s.S[i] = Assignment(t, j, i);
	      found[P.famOf[i]]++;
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
    for (int j = 0; j < P.M; ++j)
      if (P.F[f].qualif[j] && IloRound(cplex.getValue(Y[f][j])))
	for (int t = 0; t < T; ++t)
	  if (IloRound(cplex.getValue(y[f][j][t])) == 1){
	    s.QualifLostTime[f][j] = t;
	    break;
	  }
  s.repairDisqualif(P);
  return 0;
}

int solToModel(const Problem& P, const Solution& s, IloNumVar3DMatrix& x, IloNumVar3DMatrix& y, IloNumVarMatrix& Y, IloNumVarArray& C, IloNumVarArray& startVar, IloNumArray& startVal){
  const int T = P.computeHorizon();
  const int F = P.getFamilyNumber();
  const int M = P.M;

  //remplissage de x
  for (int i = 0; i < P.N; ++i){
    startVar.add(x[P.famOf[s.S[i].index]][s.S[i].machine][s.S[i].start]);
    startVal.add(1);
  }

  //remplissage de y
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < M; ++j)
      if (P.F[f].qualif[j]){
	if (s.QualifLostTime[f][j] < std::numeric_limits<int>::max()){
	  int t = 0;
	  while (t < T){
	    startVar.add(y[f][j][t]);
	    if (t < s.QualifLostTime[f][j]) startVal.add(0);
	    else startVal.add(1);
	    ++t;
	  }
	}
      }

  //remplissage de Y
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < M; ++j){
      if (P.F[f].qualif[j]){
	startVar.add(Y[f][j]);
	if (s.QualifLostTime[f][j] == std::numeric_limits<int>::max())
	  startVal.add(0);
	else startVal.add(1);
      }
    }

  //remplissage de C
  for (int f = 0; f < P.getFamilyNumber(); ++f){
    int sum = 0;
    for (int i = 0; i < P.N; ++i)
      (P.famOf[s.S[i].index] == f ? sum += s.S[i].start + P.F[f].duration : sum = sum);
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

  IloNumArray Y_ub(env, P.M);

  char namevar[24];
  for (i = 0; i < P.getFamilyNumber(); ++i){
    x[i] = IloNumVarMatrix(env, P.M);
    for (j = 0; j < P.M; ++j){
      x[i][j] = IloNumVarArray(env, T, 0, 1, ILOINT);
      for (int t = 0; t < T; ++t){
	snprintf(namevar, 24 , "x_%d_%d_%d", i, t, j);
	x[i][j][t].setName(namevar);
      }
    }
  }
  for (i = 0; i < P.getFamilyNumber(); ++i){
    y[i] = IloNumVarMatrix(env, P.M);
    for (j = 0; j < P.M; ++j){
      if (P.F[i].qualif[j]){
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
    for (int j = 0; j < P.M; ++j){
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
  const int m = P.M;
  const int F = P.getFamilyNumber();
  std::vector<int> nf(F);
  for (int q = 0; q < F; ++q)
    nf[q] = P.getNf(q);


  //objective
  IloExpr expr(env);
  for (f = 0; f < F; ++f)
    expr += alpha_C * C[f];
  for (f = 0; f < F; ++f)
    for (j = 0; j < m; ++j)
      if (P.F[f].qualif[j])
	expr += beta_Y * Y[f][j];
  model.add(IloMinimize(env, expr));
  expr.end();

  //each job is scheduled once
  for (f = 0; f < F; ++f){
    IloExpr expr(env);
    for (j = 0; j < m; ++j){
      if (P.F[f].qualif[j]){
	for (t = 0; t <= T - P.F[f].duration; ++t)
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
      if (P.F[f].qualif[j]){
	for (t = 0; t <= T - P.F[f].duration; ++t)
	  expr += ((t + P.F[f].duration) * x[f][j][t]);
      }
    }
    model.add(expr <= C[f]);
    expr.end();
  }

  //noOverlap on the same machine for to job of the same family
  for (f = 0; f < F; ++f)
    for (t = P.F[f].duration - 1; t <= T - P.F[f].duration; ++t)
      for (j = 0; j < m; ++j)
	if (P.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (tau = t - P.F[f].duration + 1; tau <= t; ++tau)
	    expr += x[f][j][tau];
	  expr += y[f][j][t];
	  model.add(expr <= 1);
	  expr.end();
	}


  //noOverlap on the same machine with setup time
  for (f = 0; f < F; ++f)
    for (int f2 = 0; f2 < F; ++f2){
      if (f != f2){
	for (t = 0/*P.F[f].duration + P.F[f2].setup - 1*/; t <= T - P.F[f2].duration; ++t)
	  for (j = 0; j < m; ++j)
	    if (!((P.F[f].qualif[j] + P.F[f2].qualif[j]) % 2)){
	      IloExpr expr(env);
	      for (tau = std::max(0,t - P.F[f].duration - P.F[f2].setup + 1);
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
      if (P.F[f].qualif[j]){
	for (t = P.F[f].threshold ; t < T; ++t){
	  IloExpr expr(env);
	  for (tau = t - P.F[f].threshold + 1; tau <= t; ++tau)
	    expr += x[f][j][tau];
	  expr += y[f][j][t];
	  model.add(expr >= 1);
	  expr.end();
	}
      }

  for (t = 1; t < T; ++t)
    for (f = 0; f < F; ++f)
      for (j = 0; j < m; ++j)
	if (P.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (int f2 = 0; f2 < F; ++f2){
	    for (tau = std::max(0 , t - P.F[f2].duration); tau < T; ++tau){
	      for (int j2 = 0; j2 < m; ++j2){
		if (P.F[f2].qualif[j2]){
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
	if (P.F[f].qualif[j])
	  model.add(y[f][j][t - 1] <= y[f][j][t]);


  return 0;
}

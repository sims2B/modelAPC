#include "CPjobModel.h"
#include <algorithm>

int CP::solve(const Problem& P, Solution & s){
  IloEnv env;
  IloNum bestObj, timeBest;
  int  solved = 0;
  try{
    IloModel model(env);
    
    IloIntervalVarArray masterTask(env, P.N + 1);
    IloIntervalVarMatrix altTasks(env, P.M);
    IloIntervalVarMatrix disqualif(env, P.M);
    IloIntervalSequenceVarArray mchs(env, P.M);
    createModel(P, env, model, masterTask, altTasks, disqualif, mchs);
    IloCP cp(model);
    cp.exportModel("model.cpo");
    if (withCPStart){
      Solution s2(P);
      if (heuristique(P, s2)){
	
	std::cout << P.toString() << s2.toString(P);
	IloSolution sol(env);
	solToModel(P, s2, masterTask, altTasks, disqualif, masterTask[P.N], sol);
	cp.setStartingPoint(sol);
	//std::cout << "sol used\n";
      }
    }
    
    //    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.setParameter(IloCP::TimeLimit, time_limit);
    //solve!
    cp.startNewSearch();
    while (cp.next()){
      if (!solved || bestObj > cp.getObjValue()){
	bestObj = cp.getObjValue();
	timeBest = cp.getInfo(IloCP::SolveTime);
      }
      solved = 1;
    }
    if (solved){
      int ret = //printSol(P,cp,altTasks,disqualif)||
	modelToSol(P, s, cp, altTasks, disqualif)
	|| displayCVS(P, s, cp, bestObj, timeBest);
      env.end();
      return ret;
    }
    else if (cp.getStatus() == IloAlgorithm::Infeasible){
      int ret = displayCVS(P, s, cp, bestObj, timeBest);
      env.end();
      return ret;
    }
  }
  
  catch (IloException &e){
    std::cout << "Iloexception in solve" << e << std::endl;
  }
  catch (...){
    std::cout << "Error unknown\n";
  }
  env.end();
  return 1;
}

int toto(Problem, Solution&){ return 1; }
int modelToSol(const Problem& P, Solution& s, const IloCP& cp,
	       const IloIntervalVarMatrix& altTasks,
	       const IloIntervalVarMatrix& disqualif){
  int i, j;
  const int m = P.M;
  const int F = P.getFamilyNumber();
  
  std::vector<int> cpt(m, 0);
  for (i = 0; i < P.N; ++i)
    for (j = 0; j < m; ++j)
      if (P.isQualif(i, j)){
	if (cp.isPresent(altTasks[j][cpt[j]])){
	  s.S[i].index = i;
	  s.S[i].start = (int)cp.getStart(altTasks[j][cpt[j]]);
	  s.S[i].machine = j;
	}
	cpt[j]++;
      }
  
  cpt.clear();
  cpt.resize(m, 0);
  for (i = 0; i < F; ++i)
    for (j = 0; j < m; ++j)
      if (P.F[i].qualif[j]){
	if (cp.isPresent(disqualif[j][cpt[j]]))
	  s.QualifLostTime[i][j] = (int)cp.getStart(disqualif[j][cpt[j]]);
	cpt[j]++;
      }
  
  return 0;
}


int solToModel(const Problem& P, const Solution& s,
	       IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
	       IloIntervalVarMatrix& disqualif, IloIntervalVar& Cmax,
	       IloSolution& sol){
  const int F = P.getFamilyNumber();
  const int M = P.M;
  
  Solution s2 = s;
  s2.reaffectId(P);
  std::sort(s2.S.begin(), s2.S.end(), idComp);
  
  std::cout << s2.toString(P);
  //masterTask
  for (int i = 0; i < P.N; ++i)
    sol.setStart(masterTask[s2.S[i].index], s2.S[i].start);
  //disqualif
  for (int j = 0; j < M; ++j){
    IloInt Fcpt = 0;
    for (int f = 0; f < F; ++f)
      if (P.F[f].qualif[j]){
	if (s2.QualifLostTime[f][j] < std::numeric_limits<int>::max())
	  sol.setPresent(disqualif[j][Fcpt]);
	else sol.setAbsent(disqualif[j][Fcpt]);
	Fcpt++;
	
      }
  }
  //Cmax
  sol.setStart(Cmax, s2.getMaxEnd(P));
  //altTask
  for (int j = 0; j < M; j++){
    IloInt Icpt = 0;
    for (int i = 0; i < P.N; ++i)
      if (P.isQualif(i, j)){
	if (s2.S[i].machine == j){
	  sol.setPresent(altTasks[j][Icpt]);
	  sol.setStart(altTasks[j][Icpt], s2.S[i].start);
	}
	else sol.setAbsent(altTasks[j][Icpt]);
	Icpt++;
      }
  }
  return 0;
}

int printSol(const Problem& P, const IloCP& cp, const IloIntervalVarMatrix& altTasks,
	     const IloIntervalVarMatrix& disqualif/*, const IloIntervalSequenceVarArray& mchs*/){
  for (int j = 0; j < P.M; ++j){
    IloInt Fcpt = 0;
    for (int f = 0; f < P.getFamilyNumber(); ++f)
      if (P.F[f].qualif[j]){
	if (cp.isPresent(disqualif[j][Fcpt]))
	  std::cout << "disQ_" << j << "=" << cp.getStart(disqualif[j][Fcpt]) << std::endl;
	Fcpt++;
      }
  }
  
  for (IloInt j = 0; j < P.M; ++j){
    for (IloInt i = 0; i < altTasks[j].getSize(); ++i){
      if (cp.isPresent(altTasks[j][i])){
	std::cout << "Task " << altTasks[j][i] << " on machine " << j << " starting at " << cp.getStart(altTasks[j][i]) << std::endl;
      }
    }
  }
  return 0;
}


int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj, const IloNum& timeBestObj){
  IloNum time_exec = cp.getInfo(IloCP::SolveTime);
  std::cout << time_exec << ";" <<timeBestObj<< ";";
  std::cout << "1;";
  if (!(cp.getStatus() == IloAlgorithm::Infeasible)){
    std::cout << cp.getObjValue() << ";" << bestObj << ";" << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P) << ";" << s.isValid(P) << ";1;";
    if (cp.getStatus() == IloAlgorithm::Optimal)
      std::cout << "1\n";
    else std::cout << "0\n";
  }
  else
    std::cout << " ; ; ; ; ; ; \n";
  return 0;
}

int createModel(const Problem& P, IloEnv& env, IloModel& model,
		IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
		IloIntervalVarMatrix& disqualif, IloIntervalSequenceVarArray& mchs){
  return createVariables(P, env, masterTask, altTasks, disqualif, mchs)
    || createConstraints(P, env, model, masterTask, altTasks, disqualif, mchs)
    || createObjective(P, env, model, masterTask, disqualif);
}

int createVariables(const Problem& P, IloEnv& env, IloIntervalVarArray& masterTask,
		    IloIntervalVarMatrix& altTasks, IloIntervalVarMatrix& disqualif,
		    IloIntervalSequenceVarArray& mchs){
  int i, j;
  const int n = P.N;
  const int m = P.M;
  const int T = P.computeHorizon();
  char name[10];
  
  for (i = 0; i < n; ++i){
    snprintf(name, 10, "master_%d", (int)i);
    masterTask[i] = IloIntervalVar(env, name);
    masterTask[i].setEndMax(T);
  }
  masterTask[n] = IloIntervalVar(env, (IloInt)0);
  masterTask[n].setEndMax(T);
  
  for (j = 0; j < m; ++j){
    altTasks[j] = IloIntervalVarArray(env);
    for (i = 0; i < n; ++i)
      if (P.isQualif(i, j)){
	IloIntervalVar alt(env, P.getDuration(i));
	snprintf(name, 10,"alt_%d_%d", (int)j, (int)i);
	alt.setName(name);
	alt.setOptional();
	altTasks[j].add(alt);
      }
  }
  
  for (j = 0; j < m; ++j){
    IloIntArray types(env, altTasks[j].getSize());
    IloInt cpt = 0;
    for (i = 0; i < n; ++i){
      if (P.isQualif(i, j)){
	types[cpt] = P.famOf[i];
	//	std::cout << "type of " << altTasks[j][cpt].getName()<< " = " << types[cpt]<< std::endl;
	cpt++;
      }
    }
    mchs[j] = IloIntervalSequenceVar(env, altTasks[j], types);
  }
  
  for (j = 0; j < m; ++j){
    disqualif[j] = IloIntervalVarArray(env);
    for (i = 0; i < P.getFamilyNumber(); ++i)
      if (P.F[i].qualif[j]){
	IloIntervalVar qual(env, (IloInt)0);
	snprintf(name,10, "disQ_%d_%d", (int)j, (int)i);
	qual.setName(name);
	qual.setOptional();
	disqualif[j].add(qual);
      }
  }
  return 0;
}

int createObjective(const Problem& P, IloEnv& env, IloModel& model,
		    IloIntervalVarArray& masterTask, IloIntervalVarMatrix& disqualif){
  IloInt i;
  IloIntExprArray ends(env);
  IloNumExprArray objs(env);
  
  //1 completion time
  for (i = 0; i < P.N; ++i)
    ends.add(IloEndOf(masterTask[i]));
  objs.add(IloSum(ends));
  
  //2 disqualif
  IloIntExprArray disQ(env);
  for (i = 0; i < P.M; ++i){
    IloInt Jcpt = 0;
    for (int j = 0; j < P.getFamilyNumber(); ++j)
      if (P.F[j].qualif[i]){
	disQ.add(((IloInt)1 - IloSizeOf(disqualif[i][Jcpt], (IloInt)1)));
	Jcpt++;
      }
  }
  objs.add(IloSum(disQ));
  
  IloMultiCriterionExpr myObj = IloStaticLex(env, objs);
  model.add(IloMinimize(env, myObj));
  //model.add(IloMinimize(env, alpha * IloSum(ends) + beta *
  //			IloSum(disQ)));
  
  ends.end();
  disQ.end();
  objs.end();
  return 0;
}

int createConstraints(const Problem& P, IloEnv& env, IloModel& model,
		      IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
		      IloIntervalVarMatrix& disqualif, IloIntervalSequenceVarArray& mchs){
  int i, j, f;
  const int n = P.N;
  const int m = P.M;
  const int F = P.getFamilyNumber();
  std::vector<int> cptMach(m, 0);
  
  // un seul "mode" est choisie pour une tâche
  for (i = 0; i < n; ++i){
    IloIntervalVarArray members(env);
    for (j = 0; j < m; ++j)
      if (P.isQualif(i, j)){
	members.add(altTasks[j][cptMach[j]]);
	cptMach[j]++;
      }
    model.add(IloAlternative(env, masterTask[i], members));
  }
  
  //setup
  IloTransitionDistance setup(env, F);
  for (i = 0; i < F; ++i)
    for (j = 0; j < F; ++j)
      (i != j ? setup.setValue(i, j, P.F[j].setup) :
       setup.setValue(i, j, 0));
  for (j = 0; j < m; j++)
    model.add(IloNoOverlap(env, mchs[j], setup, IloTrue));
  
  
  // threshold ( disqualif last executed of family on a machine)
  for (j = 0; j < m; j++){
    IloInt Fcpt = 0;
    for (f = 0; f < F; ++f)
      if (P.F[f].qualif[j]){
	IloInt Icpt = 0;
	for (i = 0; i < n; ++i)
	  if (P.famOf[i] == f){
	    model.add(IloStartBeforeStart(env, altTasks[j][Icpt],
					  disqualif[j][Fcpt], P.F[f].threshold));
	    Icpt++;
	  }
	  else if (P.isQualif(i, j)) Icpt++;
	Fcpt++;
      }
  }
  
  // threshold (if a task of f is executed on j, then an other one of f has to be
  // executed before gamma_f OR the machine j becomes disqualified for f OR no other
  // task is scheduled on j (end_j)
  for (j = 0; j < m; j++){
    IloInt Icpt = 0;
    for (i = 0; i < n; ++i){
      if (P.isQualif(i, j)){
	IloOr c(env);
	IloInt I2cpt = Icpt + 1;
	for (int i2 = i + 1; i2 < n; ++i2) {
	  if (P.isQualif(i2, j)){
	    if (P.famOf[i] == P.famOf[i2])
	      c = c || (IloPresenceOf(env, altTasks[j][I2cpt]) && IloStartOf(altTasks[j][I2cpt]) <=
			IloStartOf(altTasks[j][Icpt]) + P.getThreshold(i));
	    I2cpt++;
	  }
	}
	IloInt Fcpt = 0;
	for (f = 0; f < P.famOf[i]; ++f)
	  if (P.F[f].qualif[j]) Fcpt++;
	c = c || IloStartOf(altTasks[j][Icpt]) + P.getThreshold(i) ==
	  IloStartOf(disqualif[j][Fcpt]);
	c = c || IloStartOf(altTasks[j][Icpt]) + P.getThreshold(i) >=
	  IloStartOf(masterTask[n]);
	model.add(IloIfThen(env, IloPresenceOf(env, altTasks[j][Icpt]), c));
	Icpt++;
      }
    }
  }
  
  // the machine becomes disqualified for f OR end_j <= gamma_f
  // if there is  no task of family f executed on a qualified machine j,
  for (j = 0; j < m; j++) {
    IloInt Fcpt = 0;
    for (f = 0; f < F; ++f)
      if (P.F[f].qualif[j]) {
	IloInt Icpt = 0;
	IloOr c(env);
	for (i = 0; i < n; ++i){
	  if (P.famOf[i] == f){
	    c = c || (IloPresenceOf(env, altTasks[j][Icpt]) && IloStartOf(altTasks[j][Icpt]) <= P.F[f].threshold);
	    Icpt++;
	  }
	  else if (P.isQualif(i, j))
	    Icpt++;
	}
	c = c || (IloPresenceOf(env, disqualif[j][Fcpt]) && P.F[f].threshold ==
		  IloStartOf(disqualif[j][Fcpt]));
	c = c || IloStartOf(masterTask[n]) <= P.F[f].threshold;
	model.add(c);
	Fcpt++;
      }
  }
  
  // end_j is the last task on j
  for (j = 0; j < m; j++){
    IloInt Icpt = 0;
    for (i = 0; i < n; ++i)
      if (P.isQualif(i, j)){
	model.add(IloEndBeforeStart(env, altTasks[j][Icpt],
				    masterTask[n]));
	Icpt++;
      }
    
  }
  
  // ordonne les start des taches d'une même famille (optionnelle)
  // (version globale)
  
  for (i = 0; i < n; ++i){
    int j = i + 1;
    while (j < n && P.famOf[i] != P.famOf[j])
      ++j;
    if (j < n)
      model.add(IloStartBeforeStart(env, masterTask[i], masterTask[j]));
  }
  return 0;
}



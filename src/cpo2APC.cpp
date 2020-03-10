
#include "cpo2APC.h"

#include <algorithm>


IlcConstraint IlcRelax1SFConstraint(IloCPEngine cp, IlcIntVarArray families, IlcIntVar cardinality,
                                    IlcIntVar flowtime, IlcIntArray durations,
                                    IlcIntArray setups, IloInt propagationMask) {
  return new (cp.getHeap())
      IlcRelax1SFConstraintI(cp, families, cardinality, flowtime, durations, setups, propagationMask);
}

ILOCPCONSTRAINTWRAPPER6(IloRelax1SFConstraint, cp, IloIntVarArray, families, IloIntVar, cardinality,
                        IloIntVar, flowtime, IloIntArray, durations,
                        IloIntArray, setups, IloInt, propagationMask) {
  use(cp, families);
  use(cp, cardinality);
  use(cp, flowtime);
  return IlcRelax1SFConstraint(
      cp, cp.getIntVarArray(families), cp.getIntVar(cardinality), cp.getIntVar(flowtime),
      cp.getIntArray(durations), cp.getIntArray(setups), propagationMask);
}


void useRelax1SFConstraint(const Problem &problem, IloEnv& env,
		IloOplModel& opl, IloCP& cp, IloInt propagationMask){

  
  IloIntVarMap familyM = opl.getElement("nFamM").asIntVarMap();
  IloIntVarMap cardM = opl.getElement("nM").asIntVarMap();
  IloIntVarMap flowtimeM = opl.getElement("flowtimeM").asIntVarMap();
  IloIntMap dMap = opl.getElement("durations").asIntMap();
  IloIntMap sMap = opl.getElement("setups").asIntMap();
  IloInt nf = (IloInt) problem.getNbFams();
  IloInt nm = (IloInt) problem.getNbMchs();
  
  IloIntArray durations(env, nf); 
  IloIntArray setups(env, nf);
  if(nf > 1) {
    IloInt k = 1; 
    setups[0] = sMap.getSub( k + 1).get(k);
    durations[0] = dMap.get((IloInt) 1);
    for(IloInt i= 2; i <= nf ; i++) {
      durations[i-1] = dMap.get(i);
      setups[i-1] = sMap.getSub(k).get(i);
    }
  }
  for(IloInt i= 1; i <= nm ; i++) {
    IloIntVarArray families(env, nf);
    for(IloInt j= 1; j <= nf ; j++) {
      families[j-1] = familyM.getSub(j).get(i);
    }
    IloIntVar cardinality = cardM.get(i);
    IloIntVar flowtime = flowtimeM.get(i);
   cp.getModel().add(IloRelax1SFConstraint(env, families, cardinality, flowtime, durations, setups, propagationMask, "IloRelax1SFConstraint"));
  }
  }

void CpoSolver2APC::doSolve(IloEnv &env) {
  IloOplErrorHandler handler(env, std::cout);
  IloOplModelSource modelSource(env, config.getModelPath().c_str());
  IloOplSettings settings(env, handler);
  IloOplModelDefinition def(modelSource, settings);
  IloCP cp(env);
  IloOplModel opl(def, cp);
  MyCustomDataSource ds(env, problem, config);
  IloOplDataSource dataSource(&ds);
  opl.addDataSource(dataSource);
  opl.generate();
  configure(env, cp, config);
  for (Solution sol : solutionPool) {
    solToModel(sol, env, opl, cp);
  }
  createObj(env, opl, cp);
  if(config.withRelaxation1SF()) {
    useRelax1SFConstraint(problem, env, opl, cp, (IloInt) config.withRelaxation1SF());
  }
  IloBool solCPFound = iloSolve(cp);
  solutionCount += cp.getInfo(IloCP::NumberOfSolutions);
  if (solCPFound) {
    IloOplElement elmt = opl.getElement("mjobs");
    modelToSol(env, cp, elmt);
  }
  setStatus(cp);
  tearDown(cp);
}

void MyCustomDataSource::read() const {
  IloOplDataHandler handler = getDataHandler();
  const int F = problem.getNbFams();
  // initialize the relaxation 1SF flag
  handler.startElement("withRelaxation1SF");
  handler.addIntItem( config.withRelaxation1SF() ? 1 : 0);
  handler.endElement(); 
  // initialize the int 'simpleInt'
  handler.startElement("nbM");
  handler.addIntItem(problem.getNbMchs());
  handler.endElement();
  handler.startElement("nbF");
  handler.addIntItem(F);
  handler.endElement();

  // initialize the int array 'simpleIntArray'
  handler.startElement("fsizes");
  handler.startArray();
  for (int f = 0; f < F; ++f) {
    handler.addIntItem(problem.getNf(f));
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("durations");
  handler.startArray();
  for (int f = 0; f < F; ++f) {
    handler.addIntItem(problem.getDuration(f));
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("thresholds");
  handler.startArray();
  for (int f = 0; f < F; ++f) {
    handler.addIntItem(problem.getThreshold(f));
  }
  handler.endArray();
  handler.endElement();

  // initialize a 2-dimension int array 'a2DIntArray'
  handler.startElement("setups");
  handler.startArray();
  for (int f = 0; f < F; f++) {
    handler.startArray();
    for (int j = 0; j < F; j++)
      if (f == j)
        handler.addIntItem(0);
      else
        handler.addIntItem(problem.getSetup(j));
    handler.endArray();
  }
  handler.endArray();
  handler.endElement();

  handler.startElement("qualifications");
  handler.startArray();
  for (int f = 0; f < F; f++) {
    handler.startArray();
    for (int j = 0; j < problem.getNbMchs(); j++)
      if (problem.isQualif(f, j))
        handler.addIntItem(1);
      else
        handler.addIntItem(0);
    handler.endArray();
  }
  handler.endArray();
  handler.endElement();
}

void CpoSolver2APC::createObj(IloEnv &env, IloOplModel &opl, IloCP &cp) {
  IloModel model = cp.getModel();
  IloIntVar qualif = opl.getElement("qualified").asIntVar();
  IloIntVar flow = opl.getElement("flowtime").asIntVar();

  if (config.getObjectiveFunction() == "MONO") {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, flow));
    else
      model.add(IloMaximize(env, qualif));
  }

  else if (config.getObjectiveFunction() == "LEX") {
    IloNumExprArray objs(env);
    if (config.getWeightFlowtime() > config.getWeightQualified()) {
      objs.add(flow);
      objs.add(-1 * qualif);
    } else {
      objs.add(-1 * qualif);
      objs.add(flow);
    }
    IloMultiCriterionExpr myObj = IloStaticLex(env, objs);
    model.add(IloMinimize(env, myObj));
    objs.end();
  }
  else {
    if (config.getWeightFlowtime() > config.getWeightQualified())
      model.add(IloMinimize(env, flow - qualif));
    else {
      double beta = problem.getNbJobs() * problem.computeHorizon();
      model.add(IloMinimize(env, flow - beta * qualif));
    }
  }
}
void CpoSolver2APC::solToModel(const Solution &solution, IloEnv &env,
                               IloOplModel &opl, IloCP &cp) {
  IloSolution sol(env);
  std::vector<Job> solList;
  for (int m = 0; m < problem.getNbMchs(); ++m)
    for (int j = 0; j < solution.getNbJobsOn(m); ++j)
      solList.push_back(solution.getJobs(j, m));
  std::sort(solList.begin(), solList.end());

  sol.setValue((opl.getElement("flowtime")).asIntVar(),
               solution.getSumCompletion());
  sol.setEnd((opl.getElement("cmax")).asIntervalVar(), solution.getMaxEnd());

  sol.setValue((opl.getElement("qualified")).asIntVar(),
               solution.getNbQualif());

  IloIntervalVarMap jobs = opl.getElement("jobs").asIntervalVarMap();

  for (IloInt j = 1; j <= problem.getNbJobs(); ++j) {
    IloIntervalVar job_j = jobs.get(j);
    sol.setStart(job_j, solList[j - 1].getStart());
  }

  Solution copie(solution);
  IloIntervalVarMap mjobs = opl.getElement("mjobs").asIntervalVarMap();

  int deb, fin = 0;
  for (int f = 0; f < problem.getNbFams(); ++f) {
    deb = fin;
    fin = deb + problem.getNbJobs(f);
    for (IloInt j = deb; j < fin; ++j) {
      Job a(f);
      int mch;
      copie.getFirstOcc(a, f, mch);
      copie.removeJob(a, mch);
      IloIntervalVarMap sub = mjobs.getSub(j + 1);
      for (IloInt k = 0; k < problem.getNbMchs(); ++k) {
        IloIntervalVar alt_job = sub.get(k + 1);
        if (mch == k) {
          sol.setPresent(alt_job);
          sol.setStart(alt_job, a.getStart());
        } else
          sol.setAbsent(alt_job);
      }
    }
  }
  cp.setStartingPoint(sol);
}

void CpoSolver2APC::modelToSol(const IloEnv &env, const IloCP &cp,
                               const IloOplElement &elmt) {
  IloInt i, j, f;
  IloIntervalVarMap mjobs = elmt.asIntervalVarMap();
  IloIntervalVarMatrix dk(env, problem.getNbJobs());
  for (i = 0; i < problem.getNbJobs(); ++i) {
    dk[i] = mjobs[i + 1].asNewIntervalVarArray();
  }
  for (j = 0; j < problem.getNbMchs(); ++j) {
    int deb, fin = 0;
    for (f = 0; f < problem.getNbFams(); ++f) {
      deb = fin;
      fin = deb + problem.getNbJobs(f);
      if (problem.isQualif(f, j)) {
        for (i = deb; i < fin; ++i) {
          if (cp.isPresent(dk[i][j])) {
            solution.addJob(
                Job(problem.getFamily(i), (int)cp.getStart(dk[i][j]), -1), j);
          }
        }
      }
    }
  }
  solution.repairDisqualif();
}

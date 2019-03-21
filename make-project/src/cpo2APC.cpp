
#include "cpo2APC.h"

#include <algorithm>

void CpoSolver2APC::doSolve(IloEnv &env) {
  IloOplErrorHandler handler(env, std::cout);
  IloOplModelSource modelSource(env, config.getModelPath().c_str());
  IloOplSettings settings(env, handler);
  IloOplModelDefinition def(modelSource, settings);
  IloCP cp(env);
  IloOplModel opl(def, cp);
  MyCustomDataSource ds(env, problem);
  IloOplDataSource dataSource(&ds);
  opl.addDataSource(dataSource);
  opl.generate();
  configure(env, cp, config);
  // FIXME ./bin/solverAPC example.cfg
  // ../benchmarks/datasets/T4/T4_small/instance_10_2_3_10_5_Sthr_7.txt
  for (Solution sol : solutionPool) {
    solToModel(sol, env, opl, cp);
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
  IloInt i, j,f;
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

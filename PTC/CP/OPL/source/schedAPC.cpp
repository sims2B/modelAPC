#include "schedAPC.h"
#include <algorithm>

int solve(const Problem& P, Solution& s){
  IloEnv env;

  Clock::time_point startTime = Clock::now();
  //int status = 127;
  try {
    IloOplErrorHandler handler(env,std::cout);
    IloOplModelSource modelSource(env, pathToOPL);
    IloOplSettings settings(env,handler);
    IloOplModelDefinition def(modelSource,settings);
    IloCP cp(env);    
    IloOplModel opl(def,cp);
    MyCustomDataSource ds(env,P);
    IloOplDataSource dataSource(&ds);
    opl.addDataSource(dataSource);
    opl.generate();
if (!VERBOSITY)     cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.setParameter(IloCP::TimeLimit, time_limit);
    if (withCPStart){
      Solution solSCH(P);
      if (SCH(P, solSCH)) solToModel(P, solSCH, env,opl,cp);
      Solution solQCH(P);
      if (QCH(P, solQCH)) solToModel(P, solQCH, env,opl,cp);
    }
    if (cp.solve()){
      IloOplElement elmt = opl.getElement("mjobs");
      modelToSol(P,s,env,cp,elmt);
      displayCPAIOR(P, s, cp, startTime,1);
    }
    else displayCPAIOR(P, s, cp, startTime,0);
   return 0;
  } catch (IloOplException & e) {
    std::cout << "### OPL exception: " << e.getMessage() << std::endl;
  } catch( IloException & e ) {
    std::cout << "### exception: ";
    e.print(std::cout);
    return  2;
  } catch (...) {
    std::cout << "### UNEXPECTED ERROR ..." << std::endl;
    return 3;
  }

  env.end();
    
  return 127;
}


int displayCPAIOR(const Problem& P, const Solution& s, const IloCP& cp,  Clock::time_point t1, int solved){
  Clock::time_point t2 = Clock::now();
  
  if (solved) {
    if ( cp.getObjGap() >= -0.00001 && cp.getObjGap() <= 0.00001)
      std::cout << "s OPTIMUM \n";
    else std::cout << "s FEASIBLE\n";
  }
  else std::cout << "s " << cp.getStatus() << "\n";
  std::cout << "d WCTIME " <<  cp.getInfo(IloCP::SolveTime) << "\n";

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
  std::cout << "d GAP "  <<  cp.getObjGap()<< "\n";
  }
  std::cout << "d NBSOLS "  <<  cp.getInfo(IloCP::NumberOfSolutions)<< "\n";
  std::cout << "d BRANCHES " <<  cp.getInfo(IloCP::NumberOfBranches) << "\n";
  std::cout << "d FAILS "  <<  cp.getInfo(IloCP::NumberOfFails)<< "\n";
  std::cout << "c VARIABLES " <<  cp.getInfo(IloCP::NumberOfVariables) << "\n";
  std::cout << "c CONSTRAINTS " <<  cp.getInfo(IloCP::NumberOfConstraints) << "\n";
  std::cout << "c MACHINES "<< P.M << "\n";
  std::cout << "c FAMILIES "<< P.getFamilyNumber() << "\n";
  std::cout << "c JOBS "<<P.N << "\n";

  std::cout << std::endl;
  if (solved) s.toTikz(P);
 return 0;
}


void MyCustomDataSource::read() const {
  IloOplDataHandler handler = getDataHandler();
  const int F = P.getFamilyNumber();
  // initialize the int 'simpleInt'
  handler.startElement("nbM");
  handler.addIntItem(P.M);
  handler.endElement();
  handler.startElement("nbF");
  handler.addIntItem(F);
  handler.endElement();

  // initialize the int array 'simpleIntArray'
  handler.startElement("fsizes");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.getNf(f));
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("durations");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.F[f].duration);
  }
  handler.endArray();
  handler.endElement();
  handler.startElement("thresholds");
  handler.startArray();
  for (int f = 0 ; f < F ; ++f){
    handler.addIntItem(P.F[f].threshold);
  }
  handler.endArray();
  handler.endElement();

  //initialize a 2-dimension int array 'a2DIntArray'
  handler.startElement("setups");
  handler.startArray();
  for (int f = 0 ; f < F ; f++) {
    handler.startArray();
    for (int j = 0 ; j < F ; j++)
      if (f == j) handler.addIntItem(0);
      else handler.addIntItem(P.F[j].setup);
    handler.endArray();
  }  
  handler.endArray();
  handler.endElement();

  handler.startElement("qualifications");
  handler.startArray();
  for (int f = 0 ; f < F ; f++) {
    handler.startArray();
    for (int j = 0 ; j < P.M ; j++)
      if (P.F[f].qualif[j]) handler.addIntItem(1);
      else handler.addIntItem(0);
    handler.endArray();
  }  
  handler.endArray();
  handler.endElement();

}


int solToModel(const Problem& P, Solution s,
	       IloEnv& env, IloOplModel& opl, IloCP& cp){
  IloSolution sol(env);
  s.reaffectId(P);	
  std::sort(s.S.begin(),s.S.end(),idComp);
  
  sol.setValue((opl.getElement("flowtime")).asIntVar(), s.getSumCompletion(P));
  sol.setEnd((opl.getElement("cmax")).asIntervalVar(), s.getMaxEnd(P));
        
  sol.setValue((opl.getElement("qualified")).asIntVar(), s.getNbQualif(P));
	
  IloIntervalVarMap jobs = opl.getElement("jobs").asIntervalVarMap();
  for (IloInt j = 1 ; j <= P.N ; ++j){
    IloIntervalVar job_j = jobs.get(j);
    sol.setStart(job_j,s.S[j-1].start);
  }
	
  IloIntervalVarMap mjobs = opl.getElement("mjobs").asIntervalVarMap();
  for (IloInt j = 0; j < P.N ; ++j){
    IloIntervalVarMap sub = mjobs.getSub(j + 1);
    for (IloInt k = 0 ; k < P.M; ++k){
      IloIntervalVar alt_job = sub.get(k + 1);
      if (s.S[j].machine == k){
	sol.setPresent(alt_job);
	sol.setStart(alt_job, s.S[j].start);
      }
      else sol.setAbsent(alt_job);
    }
  }

  cp.setStartingPoint(sol);

  return 0;
}


int modelToSol(const Problem &P, Solution& s, const IloEnv& env, const IloCP& cp, const IloOplElement& elmt){
  IloInt i ,j;
  IloIntervalVarMap mjobs = elmt.asIntervalVarMap();
  IloIntervalVarMatrix dk(env,P.N);
  for (i = 0 ; i < P.N ; ++i){
    dk[i] = mjobs[i + 1].asNewIntervalVarArray();
  }
  
  for (i = 0; i < P.N; ++i)
    for (j = 0; j < P.M; ++j)
      if (P.isQualif(i, j)){
	if (cp.isPresent(dk[i][j])){
	  s.S[i].index = i;
	  s.S[i].start = (int)cp.getStart(dk[i][j]);
	  s.S[i].machine = j;
	}
      }
  s.repairDisqualif(P);
  return 0;
}

/*
  int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj,const IloNum& timeBestSol){return 0;}
*/

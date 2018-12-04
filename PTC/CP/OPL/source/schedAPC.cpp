#include "schedAPC.h"
#include "utils.h"
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
    if (!VERBOSITY)  cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.setParameter(IloCP::TimeLimit, time_limit);
    
      Solution solSCH(P);
      Solution solQCH(P);
      if (withCPStart)
	useCPStart(P,solSCH,solQCH,env,opl,cp);

      IloBool solCPFound = cp.solve();
      if (solCPFound){
	IloOplElement elmt = opl.getElement("mjobs");
	modelToSol(P,s,env,cp,elmt);
      }
      displayCPAIOR(P, s, solSCH, solQCH,cp, startTime,solCPFound);
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

void useCPStart(const Problem &P, Solution& solSCH, Solution& solQCH,IloEnv& env,
		IloOplModel& opl, IloCP& cp){ 
  int nbHSol = 0;
  if (SCH(P, solSCH)) {
    solToModel(P, solSCH, env,opl,cp);
    nbHSol++;
  }
  else solSCH.clear(P);
  if (QCH(P, solQCH)){
    solToModel(P, solQCH, env,opl,cp);
    nbHSol++;
  }
  else solQCH.clear(P);
  std::cout << "d NBHSOLS "  << nbHSol << "\n";
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
	//	sol.setStart(alt_job, s.S[j].start);
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

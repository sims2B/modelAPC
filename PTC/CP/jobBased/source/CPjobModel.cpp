#include "CPjobModel.h"

int CP::solve(const Problem& P, Solution & s){
  IloEnv env;
  try{
    IloModel model(env);
    
    IloIntervalVarArray masterTask(env,P.N);
    IloIntervalVarMatrix altTasks(env, P.M);
    IloIntervalVarMatrix disqualif(env, P.M);
    IloIntervalSequenceVarArray mchs(env,P.M);
    createModel(P,env,model,masterTask,altTasks,disqualif,mchs);
    IloCP cp(model);
    
    cp.setParameter(IloCP::TimeLimit, 500);
    if (cp.solve()) {
      cp.out() << "Objective \t: " << cp.getObjValue() << std::endl;
      modelToSol(P,s,cp,altTasks,disqualif);
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}

int modelToSol(const Problem& P, Solution& s, const IloCP& cp,
	       const IloIntervalVarMatrix& altTasks,
	       const IloIntervalVarMatrix& disqualif){
  IloInt i , j ;
  const int m = P.M;
  const int F = P.getFamilyNumber();
  
  std::vector<int> cpt(m,0);
  for (i = 0 ; i < P.N ;++i)
    for (j = 0 ; j < m ; ++j)
      if (P.isQualif(i,j)){
	if (cp.isPresent(altTasks[j][cpt[j]])){
	  s.S[i].index = i;
	  s.S[i].start = cp.getStart(altTasks[j][cpt[j]]);
	  s.S[i].machine = j;
	}
	cpt[j]++;
      }
  
  cpt.clear();
  cpt.resize(m,0);
  for (i = 0 ; i < F ; ++i) 
    for (j = 0 ; j < m ; ++j)
      if (P.F[i].qualif[j]){
	if (cp.isPresent(disqualif[j][cpt[j]]))
	  s.QualifLostTime[i][j] = cp.getStart(disqualif[j][cpt[j]]);
	cpt[j]++;
      }
  
  return 0;
}

int printSol(){
        /*      for (int j = 0 ; j < P.M ; ++j){
	IloInt Fcpt = 0;
	for (int f = 0 ; f < P.getFamilyNumber() ; ++f)
	  if (P.F[f].qualif[j]){
	    if (cp.isPresent(disqualif[j][Fcpt]))
	      std::cout << "disQ_" << j<< "="<< cp.getStart(disqualif[j][Fcpt])<<std::endl;
	    Fcpt++;
	  }
	std::cout << "end_" << j<< "="<< cp.getStart(altTasks[j][altTasks[j].getSize()-1])<<std::endl;
      }

 for (IloInt j = 0 ; j < P.M ; ++j){
	  for (IloInt i = 0 ; i < mchs[j].getSize() ; ++i){
	  if (mchs[j][i].isPresent()){
	  std::cout << "Task " << m.n << " on machine " << j << " starting at " << mchs[j][i].start << std::endl;}}*/
  return 0;
}

int createModel(const Problem& P, IloEnv& env, IloModel& model,
		IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
		IloIntervalVarMatrix& disqualif, IloIntervalSequenceVarArray& mchs){
  return createVariables(P,env,masterTask,altTasks,disqualif,mchs)
    || createConstraints(P,env,model,masterTask,altTasks,disqualif,mchs)
    || createObjective(P,env,model,masterTask,disqualif);
}
 
int createVariables(const Problem& P, IloEnv& env, IloIntervalVarArray& masterTask,
		    IloIntervalVarMatrix& altTasks, IloIntervalVarMatrix& disqualif,
		    IloIntervalSequenceVarArray& mchs){
  IloInt i,j;
  const int n = P.N;
  const int m = P.M;
  const int T = P.computeHorizon();
  char name[10];
  
  for (i = 0 ; i < n ; ++i){
    sprintf(name,"t_%d" , (int)i);
    masterTask[i] = IloIntervalVar(env,name);
    masterTask[i].setEndMax(T);
  }
  for (j = 0 ; j < m ; ++j){
    altTasks[j] = IloIntervalVarArray(env);
    for (i = 0 ; i < n ; ++i)
      if (P.isQualif(i,j)){
	IloIntervalVar alt(env,P.getDuration(i));
	sprintf(name,"alt_%d_%d" , (int)j , (int) i);
	alt.setName(name);
	alt.setOptional();
        altTasks[j].add(alt);
      } }

  for (j = 0 ; j < m ; ++j){
    IloIntArray types(env,altTasks[j].getSize());
    IloInt cpt=0;
    for (i = 0 ; i < n ; ++i){
      if (P.isQualif(i,j)){
	types[cpt]=P.famOf[i];
	cpt++;
      }
    }
    mchs[j]= IloIntervalSequenceVar(env , altTasks[j] , types );
  }

  for (j = 0 ; j < m ; ++j){
    IloIntervalVar alt(env,(IloInt)0);
    sprintf(name,"end_%d" , (int)j);
    alt.setName(name);
    altTasks[j].add(alt);
  }
  
  for (j = 0 ; j < m ; ++j){
    disqualif[j] = IloIntervalVarArray(env);
    for (i = 0 ; i < P.getFamilyNumber() ; ++i)
      if (P.F[i].qualif[j]){
	IloIntervalVar qual(env,(IloInt)0);
	sprintf(name,"disQ_%d_%d" , (int)j , (int)i);
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
  
  for (i = 0 ; i < P.N ; ++i)
    ends.add(IloEndOf(masterTask[i])); 
  
  IloIntExprArray disQ(env);
  for (i = 0 ; i < P.M ; ++i){
    IloInt Jcpt = 0;
    for (int j = 0 ; j < P.getFamilyNumber() ; ++j)
      if (P.F[j].qualif[i]){
	disQ.add(((IloInt)1 - IloSizeOf(disqualif[i][Jcpt],(IloInt)1)));
	Jcpt++;
      }
  }
  model.add(IloMinimize(env, alpha * IloSum(ends) + beta *
						    IloSum(disQ)));

  ends.end();
  disQ.end();
  return 0;
}

int createConstraints(const Problem& P, IloEnv& env, IloModel& model,
		      IloIntervalVarArray& masterTask, IloIntervalVarMatrix& altTasks,
		      IloIntervalVarMatrix& disqualif, IloIntervalSequenceVarArray& mchs){
  IloInt i, j , f;
  const int n = P.N;
  const int m = P.M;
  const int F = P.getFamilyNumber();
  std::vector<int> cptMach(m,0);

  // un seul "mode" est choisie pour une tâche
  for (i = 0 ; i < n ; ++i){
    IloIntervalVarArray members(env);
    for (j = 0 ; j < m ; ++j)
      if (P.isQualif(i,j)){
	members.add(altTasks[j][cptMach[j]]);
	cptMach[j]++;
      }
    model.add(IloAlternative(env,masterTask[i],members));
  }

  //setup
  IloTransitionDistance setup(env,F);
  for (i = 0 ; i < F ; ++i)
    for (j = 0 ; j < F ; ++j)
      ( i != j ? setup.setValue(i,j,P.getSetup(j)) :
	setup.setValue(i,j,0) );
  for (j = 0 ; j < m ; j++)
    model.add(IloNoOverlap(env, mchs[j] , setup , IloTrue));
  

 
  // threshold ( disqualif last executed of family on a machine)
  for (j = 0 ; j < m ; j++){
    IloInt Fcpt = 0;
    for (f = 0 ; f < F ; ++f)
      if (P.F[f].qualif[j]){
	IloInt Icpt = 0;
	for (i = 0 ; i < n ; ++i)
	  if (P.famOf[i] == f){
	    model.add(IloEndBeforeStart(env,altTasks[j][Icpt],
					disqualif[j][Fcpt],P.F[f].threshold));
	    Icpt++;
	  }
	  else if (P.isQualif(i,j)) Icpt++;
	Fcpt++;
      }
  }
  
   
   // threshold (if a task of f is executed on j, then an aother one of f has to be
  // executed before gamma_f OR the machine j becomes disqualified for f OR no other
  // task is scheduled on j (end_j)
   for (j = 0 ; j < m ; j++){
     IloInt Icpt = 0;
     for (i = 0 ; i < n ; ++i){
       if (P.isQualif(i,j)){
	 IloOr c(env);
	 IloInt I2cpt = Icpt+1;
	 for (IloInt i2 = i+1 ; i2 < n ; ++i2) {
	   if (P.isQualif(i2,j)){
	     if (P.famOf[i]==P.famOf[i2])
	       c = c || (IloPresenceOf(env, altTasks[j][I2cpt]) && IloStartOf(altTasks[j][I2cpt]) <=
			 IloEndOf(altTasks[j][Icpt])+ P.getThreshold(i));
	     I2cpt++;
	   }
	 }
	 IloInt Fcpt = 0;
	 for (f = 0 ; f < P.famOf[i] ; ++f)
	   if (P.F[f].qualif[j]) Fcpt++;
	 c = c || IloEndOf(altTasks[j][Icpt]) +  P.getThreshold(i) ==
	   IloStartOf(disqualif[j][Fcpt]);
	 c = c ||  IloEndOf(altTasks[j][Icpt]) ==
	   IloStartOf(altTasks[j][altTasks[j].getSize()-1]);
	 model.add(IloIfThen(env,IloPresenceOf(env,altTasks[j][Icpt]), c));
	 Icpt++;
       }	 
     }
   }
  // end_j is the last task on j
   for (j = 0 ; j < m ; j++){
     IloInt Icpt = 0;
     for (i = 0 ; i < n ; ++i)
       if (P.isQualif(i,j)){
	 model.add(IloEndBeforeStart(env,altTasks[j][Icpt],
				     altTasks[j][altTasks[j].getSize()-1]));
	 Icpt++;
       }

   }
   // if there is  no task of family f executed on a qualified machine j,
   // the machine becomes disqualified for f OR end_j <= gamma_f
   for (j = 0 ; j < m ; j++) {
     IloInt Fcpt = 0;
     for (f = 0 ; f < F ; ++f) 
       if (P.F[f].qualif[j]) {
	 IloInt Icpt = 0;
	 IloOr c(env);
	 for (i = 0 ; i < n ; ++i)
	   if (P.famOf[i]==f){
	     c = c || IloPresenceOf(env, altTasks[j][Icpt]);
	     Icpt++;
	   }
	   else if (P.isQualif(i,j)) Icpt++;
	 c = c || IloPresenceOf(env, disqualif[j][Fcpt]);
	 c = c || IloStartOf(altTasks[j][altTasks[j].getSize()-1]) <= P.F[f].threshold;
	 model.add(c);
	 Fcpt++;
       }	 
   }
   
 

   // ordonne les start des taches d'une même famille (optionnelle)
   // (version globale)
 
   for (i = 0 ; i < n ; ++i){
     int j = i + 1 ;
     while ( j < n && P.famOf[i]!=P.famOf[j])
       ++j;
     if (j < n )
       model.add(IloStartBeforeStart(env,masterTask[i],masterTask[j]));
   }
   return 0;
}


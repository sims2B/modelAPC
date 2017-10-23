#include "familyModel.h"
#include <cmath>
int solve(const Problem& P, Solution& s){
  IloNum start;
  const int F = P.getFamilyNumber();
  const int T = P.computeHorizon();
  IloEnv env;
  IloModel model(env);

  IloNumVar3DMatrix x(env,F);
  IloNumVar3DMatrix y(env,F);
  IloNumVarArray C(env,F,0,T+1,ILOFLOAT);

  if (!createModel(P,T,env,model,x,y,C)){
    IloCplex cplex(model);
    
    setParam(env,cplex);

    start=cplex.getCplexTime();

    //solve!
    if (cplex.solve() || cplex.getStatus()==IloAlgorithm::Infeasible){
      int ret = displayCplexResults(cplex,start);
	//|| displayCplexSolution(P,env,cplex,x,y,C);
      if (cplex.getStatus()==IloAlgorithm::Infeasible){
	env.end();
	return ret;
      }
      else {
	ret = (ret || modelToSol(P,s,cplex,x,y));
	env.end();
	return ret;
      }
    }
  }
  env.end();
  return 1;
}
    
int displayCplexSolution(const Problem& P, IloEnv& env, IloCplex& cplex, const IloNumVar3DMatrix& x, const IloNumVar3DMatrix& y, const IloNumVarArray& C){
  IloNumArray v(env);

  int f,j;
  const int F=P.getFamilyNumber();
  // affichage de x et C
  for (f = 0 ; f < F ; ++f) {
    for (j = 0 ; j < P.M ; ++j)
      if (P.F[f].qualif[j]){
	IloInt t = 0;
	while (t < x[f][j].getSize() - P.F[f].duration &&
	       IloRound(cplex.getValue(x[f][j][t]))!= 1)
	  ++t;
	if (t < x[f][j].getSize() - P.F[f].duration){
	  std::cout << "x["<<f<<"]["<<j<<"]=";
	    for (IloInt tau = 0 ; tau < x[f][j].getSize() - P.F[f].duration ; ++tau)
	      std::cout << cplex.getValue(x[f][j][tau]) << " , " ;
	    std::cout << std::endl;
	  }
      }
    std::cout<< "\n C["<<f<<"]="<<cplex.getValue(C[f])<<std::endl;
  }
  
  for (f = 0 ; f < F ; ++f) 
    for (j = 0 ; j < P.M ; ++j) 
      if (P.F[f].qualif[j]){
	cplex.getValues(y[f][j],v);
	std::cout << "y["<<f<<"]["<<j<<"]="<<v <<std::endl;
      }
	
  return 0;
}

int displayCplexResults(const IloCplex& cplex, const IloNum& start){
  IloNum time_exec=cplex.getCplexTime() - start;
  std::cout << "Final status: \t"<< cplex.getStatus() << " en " 
	    << time_exec << std::endl;
  if (!(cplex.getStatus()==IloAlgorithm::Infeasible)){
    std:: cout << "Final objective: " << cplex.getObjValue() 
	       <<"\nFinal gap: \t" << cplex.getMIPRelativeGap()
	       << std::endl;
  }
  return 0;
}

int setParam(IloEnv& env,IloCplex& cplex){
  cplex.setParam(IloCplex::TiLim, time_limit);
  cplex.setParam(IloCplex::Threads,2);
  cplex.setOut(env.getNullStream());
  return 0;
}

int modelToSol(const Problem& P, Solution& s, IloCplex& cplex, IloNumVar3DMatrix& x,
	       IloNumVar3DMatrix& y){

  const int F = P.getFamilyNumber(); 
  const int T = P.computeHorizon();
  std::vector<int> found(F,0);
  
  for (int i = 0 ; i < P.N ; ++i){   
    int cpt=0;
    int j=0;
    int trouve=0;
    while (j < P.M && !trouve){
      if (P.isQualif(i,j)){
	int t=0;
	while (t < T - P.getDuration(i) && !trouve){
	  if (IloRound(cplex.getValue(x[P.famOf[i]][j][t]))==1){
	    ++cpt;
	    if (cpt == found[P.famOf[i]]+1){
	      s.S[i]=Assignment(t,j);
	      found[P.famOf[i]]++;
	      trouve=1;
	    }
	  }
	++t;
	}
      }
    ++j;
    }
  }
  
  for (int f = 0 ; f < F ; ++f)
    for (int j = 0 ; j < P.M ; ++j)
      if (P.F[f].qualif[j])
	for (int t = 0 ; t < T ; ++t)
	  if (IloRound(cplex.getValue(y[f][j][t]))==1)
	    s.QualifLostTime[f][j]=t;
  return 0;
}

int createModel(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		IloNumVar3DMatrix& y, IloNumVarArray& C){
  return createVars(P,T,env,x,y) || createConstraints(P,T,env,model,x,y,C);
}

int createVars(const Problem& P, int T, IloEnv& env, IloNumVar3DMatrix& x,
	       IloNumVar3DMatrix& y){
  IloInt i,j;

 
  for (i = 0 ; i < P.getFamilyNumber() ; ++i){
    x[i] = IloNumVarMatrix(env,P.M);
    for (j = 0 ; j < P.M ; ++j)
      x[i][j] = IloNumVarArray(env,T,0,1,ILOINT);
  }
  for (i = 0 ; i < P.getFamilyNumber() ; ++i){
    y[i] = IloNumVarMatrix(env,P.M);
    for (j = 0 ; j < P.M ; ++j){
      (P.F[i].qualif[j] ? y[i][j] = IloNumVarArray(env,T,0,1,ILOINT) :
       y[i][j] = IloNumVarArray(env,T,0,0,ILOINT));
    }
  }
  return 0;
}

int createConstraints(const Problem& P, int T, IloEnv& env, IloModel& model, IloNumVar3DMatrix& x,
		      IloNumVar3DMatrix& y, IloNumVarArray& C){
  IloInt j,t,f,tau;
  const int m = P.M;
  const int F = P.getFamilyNumber(); 
  std::vector<int> nf(F);
  for (int q = 0 ; q < F ; ++q)
    nf[q]=P.getNf(q);
  
  
  //objective
   IloExpr expr(env);
  for (f = 0 ; f < F ; ++f)
    expr += alpha * C[f];
  for (f = 0 ; f < F ; ++f)
    for (j = 0 ; j < m ; ++j)
      if (P.isQualif(f,j))
	expr+= beta * y[f][j][T-1];
  model.add(IloMinimize(env,expr));
  expr.end();
  
  //each job is scheduled once
  for (f = 0 ; f < F ; ++f){
    IloExpr expr(env);
    for (j = 0 ; j < m ; ++j){
      if (P.F[f].qualif[j]){
	for (t = 0 ; t < T - P.F[f].duration ; ++t)
	  expr+=x[f][j][t];
      }
    }
    model.add(expr==nf[f]);
    expr.end();
  }  
  
  //completion time of a job
  for (f = 0 ; f < F ; ++f){
    IloExpr expr(env); 
    for (j = 0 ; j < m ; ++j){
      if (P.F[f].qualif[j]){
	for (t = 0 ; t  < T - P.F[f].duration ; ++t)
	  expr+= ( (t + P.F[f].duration) * x[f][j][t] ) ;
      }
    }
    model.add(expr <= C[f]);
    expr.end();
  }
  
  //noOverlap on the same machine for to job of the same family
  for (f = 0 ; f < F ; ++f)
    for (t = P.F[f].duration ; t < T ; ++t)
      for (j = 0 ; j < m ; ++j)
	if (P.F[f].qualif[j]){
	  IloExpr expr(env);
	  for (tau = t - P.F[f].duration  ; tau < t ; ++tau)
	    expr+=x[f][j][tau];
	  model.add(expr <= 1);
	  expr.end();
	}
  

  //noOverlap on the same machine with setup time
  for (f = 0 ; f < F ; ++f)
    for (int f2 = 0 ; f2 < F ; ++f2){
      for (t = 0 ; t < T ; ++t)
	for (j = 0 ; j < m ; ++j)
	  if (f != f2 && !((P.F[f].qualif[j] + P.F[f2].qualif[j])% 2)){
	    IloExpr expr(env);
	    for (tau = std::max(0,(int)t - P.F[f].duration - P.F[f2].setup)  ;
		  tau <= t ; ++tau)
	      expr += x[f][j][tau];
	    expr += nf[f] * x[f2][j][t];
	    model.add(expr <= nf[f]);
	    expr.end();
	  }
    }   
  
  //threshold 
  for (f = 0 ; f < F ; ++f)
    for (j = 0 ; j < m ; ++j)
      if (P.F[f].qualif[j]){
	for (t = 0 ; t < P.F[f].threshold + P.F[f].duration ; ++t){
	  model.add(y[f][j][t]==0);
	}
	for (t = P.F[f].threshold + P.F[f].duration ; t < T ; ++t){
	  IloExpr expr(env);
	  for (tau = t - P.F[f].threshold - P.F[f].duration ; tau < t ; ++tau)
	    expr+=x[f][j][tau];
	  expr+=y[f][j][t];
	  //model.add(expr > 0);
	  model.add(expr >= 1);
	  expr.end();
	}
      }
  
  /*  for (f = 0 ; f < F ; ++f)
      for (j = 0 ; j < m ; ++j)
	if (P.F[f].qualif[j]){
	  for (t = P.F[f].threshold + P.F[f].duration ; t < T ; ++t){
	    IloExpr expr(env);
	    for (tau = t - P.F[f].threshold - P.F[f].duration ; tau < t ; ++tau)
	      expr+=x[f][j][tau];
	    expr= expr/P.F[f].threshold;
	    expr+=y[f][j][t];
	    model.add(expr <= 1);
	    expr.end();
	  }
	}
  */

  //empeche deux tache de la meme famille de s'exécuter dans un intervalle
  //de taille gamma_f
  
  //if a machine become disqualified, it stays disqualified
  for (t = 1 ; t < T ; ++t)
    for (f = 0 ; f < F ; ++f)
      for (j = 0 ; j < m ; ++j)
	if (P.F[f].qualif[j])
	  model.add(y[f][j][t-1] <= y[f][j][t]);
  
  	    
    return 0;
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "Solution.h"
#include "SchedulingCentric.h"
#include "QualifCentric.h"
#include "paramModelAPC.h"
#include <ilopl/iloopl.h>

#ifndef SCHEDAPC_H
#define SCHEDAPC_H


typedef IloArray<IloIntervalVarArray> IloIntervalVarMatrix;
typedef IloArray<IloOplElement> IloOplElementArray;


class MyCustomDataSource : public IloOplDataSourceBaseI {
  Problem P;
public:
  MyCustomDataSource(IloEnv& env,const Problem& _P) : IloOplDataSourceBaseI(env), P(_P) {};
  void read() const;
};


int solve(const Problem&, Solution&);


//REMPLISSAGE STRUCTURE SOLUTION A PARTIR DU MODELE
int modelToSol(const Problem &, Solution&, const IloEnv&, const IloCP&,const IloOplElement&);

//A PARTIR D'UN OBJET SOLUTION, CREE UNE SOLUTION POUR LE MODELE CP (ET LUI DONNE)
int solToModel(const Problem& P, Solution s, IloEnv&, IloOplModel&, IloCP&);


//AFFICHAGE SOLUTION
int displayCPAIOR(const Problem&, const Solution& , const IloCP&, Clock::time_point,
	int solved);
/*
int displayCVS(const Problem& P, const Solution& s, const IloCP& cp, const IloNum& bestObj,const IloNum& timeBestSol);
*/
    
  
#endif

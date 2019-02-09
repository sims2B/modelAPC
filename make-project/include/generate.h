#ifndef GENERATE_H
#define GENERATE_H

#include "problem.h"

///////////////////////////////////////////////////
/////////////////// generateAli ///////////////////
///////////////////////////////////////////////////
Problem generateProblem(const int& n, const int& m, const int& F, 
						const int& pmax, int sumQualif);
void generateFamilies(Problem& P, const int& m, const int& F, const int& pmax, 
					int sumQualif);
//distribution uniforme [1,pmax]
void generateDuration(Problem& P,const int& F,const int& pmax);
//distribution uniforme [1,pmin]
void generateSetup(Problem& P, const int& F, const int& pmin);
void generateQualif(Problem& P, const int& m, const int& F, int sumQualif);
void generateThreshold(Problem& P,const int& m, const int& F,const int& pmax, 
						const int& smax);
///////////////////////////////////////////////////
///////////////////// common //////////////////////
///////////////////////////////////////////////////
void affectFamily(Problem& P, const int& n, const int& F);

///////////////////////////////////////////////////
////////////////// generateAbdoul /////////////////
///////////////////////////////////////////////////

Problem generateProblem(const int& n, const int& m, const int& F);
void generateFamilies(Problem& P,const int& m,const int& F);
void generateThreshold(Problem& P, const int& F, const int& Hbound);
void generateThreshold(Problem& P,const int& m, const int& F);
void generateDuration(Problem& P,const int& F);
void generateSetup(Problem& P, const int& F);
void generateQualif(Problem& P, const int& m, const int& F);

#endif

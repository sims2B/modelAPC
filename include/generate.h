#ifndef GENERATE_H
#define GENERATE_H

#include "problem.h"

// parametre pour la generation d'instance!!
// min/max duration
const int Pmax = 10;
const int Pmin = 1;
// min/max setup
const int Smax = 5;
const int Smin = 1;
// threshold
const int nbCat = 3;      // small, meduim and big
const int cat = 3;        // 1 = small, 2 = meduim , 3 = big
const int sizeMin = cat;  // "minimum threshold for at least sizeMin task"
                          // (small => 1 ; medium => 2 ; big => 3)
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

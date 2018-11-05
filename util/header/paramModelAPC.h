
#ifndef TIME_LIMIT
#define time_limit 30
#endif

#define withCPStart 1

#define weighted 0 //0 => lexico ; 1 => weighted obj value (les valeurs de alpha 
//et beta se règlent un peu plus bas)

//alpha et beta pour le weighted objectif.
#define alpha_C 1 //prio flowtime alpha = beta = 1
#define beta_Y  P.N * P.computeHorizon() //prio disqualif alpha = 1 et 
//beta = nbJob.Cmax (borne sup sur Cmax)

// lexico 
#define prioFlow 1 // ordre pour le lexico ( 1 => flow ; 0 => disqualif)


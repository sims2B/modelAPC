/*********************************************
 * OPL 12.7.1.0 Model
 * Author: Arnaud Malapert
 * Creation Date: 26 mai 2018 at 18:02:05
 *********************************************/
using CP;

 // Number of machines
 int nbM = ...;
 range M = 1..nbM;

// Number of families
int nbF = ...;
range F = 1..nbF;
  
// Number of jobs per family
int fsizes[F] = ...;
// Duration per family 
int durations[F] = ...;
// Disqualification threshold per family
int thresholds[F] = ...;
// Setup times between pair of families
int setups[F][F] = ...;
 
// qualification of machines for processing a family
int qualifications[F][M] = ...;
int qualifCount[f in F] = sum(m in M) qualifications[f][m];


 // Number of jobs
 int nbJ = sum(f in F) fsizes[f];
 range J = 1..nbJ;
 
 tuple pair {int s; int e;};
 // The range of jobs per family. 
 pair JF[f in F] = <sum(p in 1..f-1) fsizes[p] + 1, sum(p in 1..f) fsizes[p]>;
 // The family of a job
 int families[J];
  
 execute INIT_FAMILIES {
 
  	var i = 1;
 	for(var f in F) {
 		for (var j=0; j < fsizes[f]; j++) { 	
 	    	families[i++] = f;
 	    	
 	  }
 	}
}

// Scheduling horizon
//TODO Compute upper bound
int cmaxLB = ( nbM - 1 + sum(f in F) fsizes[f] * durations[f]) div nbM;
int cmaxUB = sum(f in F) fsizes[f] * (durations[f] + max(e in F) setups[f][e]);
range H = 0..cmaxUB;
range HQ = 0..cmaxUB+max(f in F) thresholds[f];
//TODO Write a test case with a single machine 

// the makespan of the schedule
dvar interval cmax in H size cmaxLB..cmaxUB; 
// the flowtime of the schedule
dvar int+ flowtime;
// The number of qualifications at the makespan
dvar int+ qualified;
dexpr int disqualified = sum(f in F, m in M) qualifications[f][m] - qualified;
 // the jobs to schedule
 dvar interval jobs[j in J] in H size durations[families[j]];
 // optional variables that model the different execution modes of a job, 
 // the different machines on which it can be scheduled
 dvar interval mjobs[j in J][M] optional in H size durations[families[j]];
 
// sequence variables that represent the unary machines  
 dvar sequence machines[m in M] in all(j in J) mjobs[j][m] types families;
 
 // cumul function that is a relaxation of the parallel machines 
 cumulFunction cumProfile = sum(j in J) pulse(jobs[j], 1);
 
 // The type triplet is required for transition times  
 tuple triplet {int id1; int id2; int value;};
 // Setup times between two jobs of different families
 {triplet} setupTimes = { <i, j, setups[i][j]> | i in F, j in F};
 
 
// A qualification job represents the time interval immediatly following the job 
// during which the machine is qualified to start another job of the same family.  
dvar interval qjobs[j in J][M] optional in HQ size thresholds[families[j]];

// The qualification profile is greater than 1 if the machine m is qualified for the family f.
cumulFunction qualifProfiles[f in F][m in M] = pulse(0, thresholds[f] + durations[f], 1) + 
                                            	sum(j in J : families[j] == f) pulse(qjobs[j][m], 1); 

// A dummy job that indicates that the machine is qualified for a family at Cmax. 
//TODO Restrict domain
dvar interval qcmax[F][M] optional in HQ size HQ;

///////////////////////////////
// Search Configuration ///////
///////////////////////////////
int timeLimit = 180;
string logVerbosity = "Normal";

float wctime;


execute SEARCH {
        cp.param.logperiod=10000;
        //cp.param.logperiod=1;
        cp.param.LogVerbosity = logVerbosity;
        //cp.param.LogVerbosity = "Quiet";
        if(timeLimit > 0) {cp.param.timelimit=timeLimit;}
        // var f = cp.factory;
        //TODO ? cp.setSearchPhases( ...)));
        //cp.param.workers=1;
        wctime = (new Date()).getTime();

    	//var f = cp.factory;
  		//cp.setSearchPhases(f.searchPhase(mjobs)); 
}

//minimize staticLex(-qualified, flowtime);

minimize flowtime - qualified;
//minimize flowtime;

subject to {
		
		ctCmax:
		span(cmax, jobs); 
		
		ctSCmax:
		startOf(cmax) == 0;					
 		
 		ctFlow:
		flowtime == sum(j in J) endOf(jobs[j]);
		
		ctQual:
		qualified == sum(f in F, m in M) presenceOf(qcmax[f][m]);		
 		 
 		ctQualLB:
 		qualified >= 1;
 		
 		 // Assignement to parallel machines
 		forall(j in J) {
 			ctAssign:  		
 			alternative(jobs[j], all(m in M) mjobs[j][m]);	 		
 		}
 		
 		// NonOverlap constraints with setup times for machines  
 		forall(m in M) { 	
 	 		ctMachines: 
 	 		  noOverlap(machines[m], setupTimes); 		 
 		 }
 		 
 		// Cumulative relaxation constraint
 		ctRelax: 
 		cumProfile <= nbM;
 	 	
 	 	// Precedence constraints between jobs of the same family
 	 	forall(f in F) {
 			forall(j in JF[f].s+1..JF[f].e) {
 				// Weak ordering of jobs that can be executed in parallel
 				startBeforeStart(jobs[j-1], jobs[j]);
 				// Maximum time lag between two jobs 
 				startBeforeStart(jobs[j], jobs[j-1], -thresholds[f]);	
 				 			
 			}
 			forall(j in JF[f].s+qualifCount[f]..JF[f].e) {
 				 	// Total ordering of jobs that cannot be executed in parallel.
 				 	// Indeed, there are at least Mf jobs between them.
 				 	// Mf is the number of machines for which the job is qualified.
 				 	endBeforeStart(jobs[j-qualifCount[f]], jobs[j]);	
 			}				
 		}
 		
 	 	// Precedence constraints between jobs of the same family processed by the same machine 
 	 	forall(m in M) {
 	 		forall(f in F) {
 				forall(j in JF[f].s+1..JF[f].e) {
 				// Weak ordering of qualification jobs
 				startBeforeStart(qjobs[j-1][m], qjobs[j][m], durations[f]); 
 					forall(i in JF[f].s..j-1) { 				
 						// Total ordering of jobs of the same family
 						before(machines[m], mjobs[i][m], mjobs[j][m]);				 	
 				 	}		
 				}		
 			}				
 		}
 		
 		// Some machines are not qualified to process a family
		forall(f in F) {
			forall(m in M : qualifications[f][m] == 0) {		
				ctMQualif:
				presenceOf(qcmax[f][m]) == 0;
				forall(j in JF[f].s..JF[f].e) {
					presenceOf(mjobs[j][m]) == 0;
					presenceOf(qjobs[j][m]) == 0;			
				}
			}
		}
				
		// The machine is qualified to process a job if
		// its qualification profile is greater than 1.
		forall(f in F) {
 				forall(j in JF[f].s..JF[f].e) {
 					forall(m in M) {
 				  		alwaysIn(qualifProfiles[f][m], mjobs[j][m], 1, fsizes[f] + 1); 				
 				}
 			}				
 		}
 		
		// The qualification job starts at the end of the machine job. 
 		forall(j in J) {
 			forall(m in M) { 
 			    ctMQPres:		
 				presenceOf(mjobs[j][m]) == presenceOf(qjobs[j][m]);		
 				ctMQPrec:
 				endAtStart(mjobs[j][m], qjobs[j][m]);
    		} 	
    	}    		
			
    	// The machine is qualified if it can process a job at the makespan.		
 		forall(f in F) {
			forall(m in M) {
				ctSQCmax:			
					startOf(qcmax[f][m]) == 0;
				ctEQCmax:
					endAtEnd(cmax, qcmax[f][m], durations[f]);
				ctQCmax:								
		  			alwaysIn(qualifProfiles[f][m], qcmax[f][m], 1, fsizes[f] + 1);  	
		  			// hmax is the number of jobs in the family plus the qualification pulse at the beginning.
		  			
			}
 		 } 		 

}


execute POSTPROCESS{
    // http://www-01.ibm.com/support/knowledgecenter/SS6MYV_3.3.0/ilog.odms.ide.odme.help/html/refjavaopl/html/ilog/cp/IloCP.IntInfo.html  
    
        // This information gives the fail status of the invoking IloCP instance. 
        // After a call to IloCP::solve or IloCP::next, you can interrogate the fail status, which can be one of the following values:
        //
        //    IloCP::SearchHasNotFailed indicates that the search has not failed.
        //    IloCP::SearchHasFailedNormally indicates that the search has failed because it has searched the entire search space.
        //    IloCP::SearchStoppedByLimit indicates that the search was stopped by a limit, such as a time limit (see IloCP::TimeLimit) or a fail limit (see IloCP::FailLimit).
        //    IloCP::SearchStoppedByLabel indicates that the search was stopped via a fail label which did not exist on any choice point (advanced use).
        //    IloCP::SearchStoppedByExit indicates that the search was exited using IloCP::exitSearch.
        //    IloCP::SearchStoppedByAbort indicates that the search was stopped by calling IloCP::abortSearch
        //    IloCP::UnknownFailureStatus indicates that the search failed for some other reason.
        var SearchHasNotFailed=12;
        var SearchHasFailedNormally=13;
        var SearchStoppedByLimit=14;
        //var SearchStoppedByLabel=15;
        //var SearchStoppedByExit=16;
        //var SearchStoppedByAbort=17;
        //var SearchStoppedByException=18;
        //var UnknownFailureStatus=19;

        if(cp.info.FailStatus == SearchHasNotFailed) {
                writeln("s SAT");
        } else if(cp.info.FailStatus == SearchHasFailedNormally) {
                writeln("s OPTIMUM");  
        } else if(cp.info.FailStatus == SearchStoppedByLimit) {
                writeln("s TIMEOUT");  
        } else {
                writeln("s ERROR"); 
    }           
    
      wctime = Opl.round((new Date()).getTime() - wctime)/1000;
      writeln("d WCTIME ", wctime);
    
  	
        writeln("d RUNTIME ", cp.info.TotalTime);
        writeln("d CMAX ", Opl.endOf(cmax));
        writeln("d FLOWTIME ", flowtime);
        writeln("d DISQUALIFIED ", disqualified);
        writeln("d QUALIFIED ", qualified);
        writeln("d NBSOLS ", cp.info.NumberOfSolutions);
        writeln("d BRANCHES ", cp.info.NumberOfBranches);
        writeln("d FAILS ", cp.info.NumberOfFails);
        writeln("c VARIABLES ", cp.info.NumberOfVariables);
        writeln("c CONSTRAINTS ", cp.info.NumberOfConstraints);
        writeln("c MACHINES ", nbM);
        writeln("c FAMILIES ", nbF);
        writeln("c JOBS ", nbJ);
}



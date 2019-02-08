#include "solTreat.h"
#include "utils.h"
#include <algorithm>

std::string getStatus(const IloBool& solFound, const IloCP& cp ){
  if (solFound){
    if (isNul(cp.getObjGap())) return "OPTIMUM";
	else return "SAT";
  }
  else{
    if (cp.getInfo(IloCP::SearchStatus) == IloCP::SearchCompleted) return "UNSAT";
    else if (cp.getInfo(IloCP::SearchStatus) == IloCP::SearchStopped) return "UNKNOWN";
    else return "ERROR";
  }
}




int displayCPAIOR(const Problem& P, const Solution& s, const Solution& solSCH, const Solution& solQCH, const IloCP& cp,  Clock::time_point t1, const IloBool& solved){
  Clock::time_point t2 = Clock::now();
  std::cout << "s " << getStatus(solved,cp) << "\n";
  
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
  int objSolCP = s.getWeigthedObjectiveValue(P);
  if (objSolCP == solSCH.getWeigthedObjectiveValue(P) ||
      objSolCP == solQCH.getWeigthedObjectiveValue(P)  )
    std::cout << "d NBSOLS 0 \n";
  else
    std::cout << "d NBSOLS "  <<  cp.getInfo(IloCP::NumberOfSolutions)<< "\n";
  }
  else std::cout << "d NBSOLS 0 \n";
  std::cout << "d BRANCHES " <<  cp.getInfo(IloCP::NumberOfBranches) << "\n";
  std::cout << "d FAILS "  <<  cp.getInfo(IloCP::NumberOfFails)<< "\n";
  std::cout << "c VARIABLES " <<  cp.getInfo(IloCP::NumberOfVariables) << "\n";
  std::cout << "c CONSTRAINTS " <<  cp.getInfo(IloCP::NumberOfConstraints) << "\n";
  std::cout << "c MACHINES "<< P.M << "\n";
  std::cout << "c FAMILIES "<< P.getFamilyNumber() << "\n";
  std::cout << "c JOBS "<< P.N << "\n";

  std::cout << std::endl;
  if (solved)   s.toTikz(P);
 return 0;
}

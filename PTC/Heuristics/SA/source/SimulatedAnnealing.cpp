
#include "SimulatedAnnealing.h"



int SA(const Problem& P, Solution& s, ptrHeur& computeSolInit){
  double tpsEcoule = 0.0;
  Clock::time_point t1 = Clock::now(); Clock::time_point t2;
  if (computeSolInit(P, s)){
    int t = 0;
    double currentTemp = TEMP_INIT;
    std::sort(s.S.begin(), s.S.end());
    Solution currentSol(P);
    Solution bestSol(s);
    int objBest = bestSol.getWeigthedObjectiveValue(P, alpha, beta);
    do{
      //std::cout << "temps écoulé : " << tpsEcoule << "\n temperature actuelle : " << currentTemp << "\n temperature change : " << t << std::endl;
      int cpt = 0;
      do{
	//std::cout << "\t generation of neighbor solution " << (t*NB_IT_PAR_TEMP) + cpt << std::endl;
	if (generateNeighborSol(P, s, currentSol)){
	  //std::cout << "\t \t la solution est valide ";
	  int delta;
	  if ((delta = currentSol.getWeigthedObjectiveValue(P, alpha, beta) -
	       s.getWeigthedObjectiveValue(P, alpha, beta)) <= 0){
	    s = currentSol;
	    if (currentSol.getWeigthedObjectiveValue(P, alpha, beta) <
		objBest){
	      bestSol = currentSol;
	      objBest = bestSol.getWeigthedObjectiveValue(P, alpha, beta);
	    }
	  }
	  else {
	    //	std::cout << "et pire ";
	    acceptWithProba(s, currentSol, currentTemp, delta);
	  }
	}
	else
	  //	std::cout << "\t \t la solution est fausse \n";
	  cpt++;
      } while (cpt < NB_IT_PAR_TEMP);
      t++;
      currentTemp = currentTemp * COOLING_FACTOR;
      t2 = Clock::now();
      tpsEcoule = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    } while (t < NB_TEMP_CHANGE && tpsEcoule < TIME_LIMIT);
    s = bestSol;
    return 1;
  }
  else
    return 0;
}

std::vector<std::vector<int>> computeLastOf(const Problem & P, const Solution& s){
  const int F = P.getFamilyNumber();
  int j, f, i = 0;
  std::vector<std::vector<int>> lastOf(P.M);

  for (j = 0; j < P.M; ++j){
    lastOf[j].resize(F);
    for (f = 0; f < P.getFamilyNumber(); ++f)
      (P.F[f].qualif[j] ? lastOf[j][f] = 0 : lastOf[j][f] = std::numeric_limits<int>::max());
  }
  while (i < P.N){
    lastOf[s.S[i].machine][P.famOf[s.S[i].index]] = s.S[i].start;
    ++i;
  }
  return lastOf;
}

int generateNeighborSol(const Problem& P, const Solution &s, Solution& currentSol){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> random(0, 100);
  currentSol = s;
  if (random(generator) < 50)
    return intraChange(P, s, currentSol);
  else
    return interChange(P, s, currentSol);
}

int intraChange(const Problem& P, const Solution& s, Solution& currentSol){
  // tirage machine	
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomMachine(0, P.M - 1);
  int m = randomMachine(generator);
  int nbJobs = s.getNbJobsOn(m);
  if (nbJobs > 1){
    //tirage job
    std::uniform_int_distribution<int> randomJobI(1, nbJobs);
    int i = randomJobI(generator);
    i = s.getJobs(i, m);
    std::uniform_int_distribution<int> randomJobJ(1, nbJobs - 1);
    int j = randomJobJ(generator);
    j = s.getJobs(j, m);
    if (i == j) j++;
    // intraChange move (update startTime)
    //std::cout << "before intrachange => i = " << i << " et j = " << j << " sur m = " << m << std::endl;
    if (i < j) intraChangeMoveIBeforeJ(P, s, currentSol, i, j, m);
    else if (i != j + 1) intraChangeMoveJBeforeI(P, s, currentSol, i, j, m);

    //disqualification update
    currentSol.repairDisqualif(P);
    return currentSol.isValid(P);
  }
  else return 0;
}


void startTimeUpdate(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int k){
  (P.famOf[currentSol.S[k].index] == P.famOf[s.S[j].index] ?
   currentSol.S[i].start = currentSol.S[k].start + P.getDuration(currentSol.S[k].index) :
   currentSol.S[i].start = currentSol.S[k].start + P.getDuration(currentSol.S[k].index) + P.getSetup(s.S[j].index));
}

void intraChangeMoveIBeforeJ(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int m){
  //move of j
  currentSol.S[i].index = s.S[j].index;
  if (i == 0 || s.S[i - 1].machine != m) currentSol.S[i].start = 0;
  else startTimeUpdate(P, s, currentSol, i, j, i - 1);
  // move other until ex position of j
  for (int it = i + 1; it <= j; ++it){
    currentSol.S[it].index = s.S[it - 1].index;
    startTimeUpdate(P, s, currentSol, it, it - 1, it - 1);
  }
  // move other after ex position of j
  for (int it = j + 1; it < P.N && s.S[it].machine == m; ++it)
    startTimeUpdate(P, s, currentSol, it, it, it - 1);
}

void intraChangeMoveJBeforeI(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int m){
  // move between ex j and i
  currentSol.S[j].index = s.S[j + 1].index;
  if (j == 0 || s.S[j - 1].machine != m) currentSol.S[j].start = 0;
  else startTimeUpdate(P, s, currentSol, j, j + 1, j - 1);
  for (int it = j + 1; it < i - 1; ++it){
    currentSol.S[it].index = s.S[it + 1].index;
    startTimeUpdate(P, s, currentSol, it, it + 1, it - 1);
  }
  //move of j
  currentSol.S[i - 1].index = s.S[j].index;
  startTimeUpdate(P, s, currentSol, i - 1, j, i - 2);
  // move other after ex position of j
  for (int it = i; s.S[it].machine == m && it < P.N; ++it)
    startTimeUpdate(P, s, currentSol, it, it, it - 1);
}

int interChange(const Problem& P, const Solution& s, Solution& currentSol){
  // tirage machines
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomMachineI(0, P.M - 1);
  std::uniform_int_distribution<int> randomMachineJ(0, P.M - 2);
  int mi = randomMachineI(generator);
  int mj = randomMachineJ(generator);
  if (mi <= mj) mj++;
  int nbJobsOnMi = s.getNbJobsOn(mi);
  int nbJobsOnMj = s.getNbJobsOn(mj);
  if (nbJobsOnMj > 1){
    //tirage job
    std::uniform_int_distribution<int> randomJobI(1, nbJobsOnMi);
    std::uniform_int_distribution<int> randomJobJ(1, nbJobsOnMj);
    int i = s.getJobs(randomJobI(generator), mi);
    int j = s.getJobs(randomJobJ(generator), mj);
    if (mi < mj)
      interChangeUpdateMiThenMj(P, s, currentSol, i, j, mi, mj);
    else
      interChangeUpdateMjThenMi(P, s, currentSol, i, j, mi, mj);

    currentSol.repairDisqualif(P);

  }
  return currentSol.isValid(P);
}

void interChangeUpdateMjThenMi(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj){

  int it;
  currentSol.S[j].index = s.S[j + 1].index;
  currentSol.S[j].machine = s.S[j + 1].machine;
  if (j == 0 || s.S[j - 1].machine != s.S[j + 1].machine) currentSol.S[j].start = 0;
  else startTimeUpdate(P, s, currentSol, j, j + 1, j - 1);
  for (it = j + 1; s.S[it + 1].machine == mj; ++it){
    currentSol.S[it].index = s.S[it + 1].index;
    startTimeUpdate(P, s, currentSol, it, it + 1, it - 1);
  }
  for (it=it; it < i - 1; ++it)
    currentSol.S[it] = s.S[it + 1];
  //move de j
  currentSol.S[i - 1].index = s.S[j].index;
  currentSol.S[i - 1].machine = mi;
  if (currentSol.S[i - 2].machine != mi) currentSol.S[i - 1].start = 0;
  else startTimeUpdate(P, s, currentSol, i - 1, j, i - 2);
  //update after j
  for (it = i; it < P.N && s.S[it].machine == mi; ++it){
    currentSol.S[it].index = s.S[it].index;
    startTimeUpdate(P, s, currentSol, it, it, it - 1);
  }
}
void interChangeUpdateMiThenMj(const Problem& P, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj){
  //move de j
  currentSol.S[i].index = s.S[j].index;
  if (i == 0 || s.S[i - 1].machine != mi) currentSol.S[i].start = 0;
  else startTimeUpdate(P, s, currentSol, i, j, i - 1);
  //update after j
  int it;
  for (it = i + 1; s.S[it - 1].machine == mi; ++it){
    currentSol.S[it].index = s.S[it - 1].index;
    startTimeUpdate(P, s, currentSol, it, it - 1, it - 1);
  }
  currentSol.S[it - 1].machine = mi;

  for (it =it; it <= j; ++it)
    currentSol.S[it] = s.S[it - 1];
  for (it = j + 1; it < P.N && s.S[it].machine == mj; ++it){
    currentSol.S[it].index = s.S[it].index;
    if (s.S[it - 2].machine != mj) currentSol.S[it].start = 0;
    else startTimeUpdate(P, s, currentSol, it, it, it - 1);
  }

}

void acceptWithProba(Solution& s, const Solution& currentSol, const double currentTemp, const int delta){

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<double> randomSelector(0.0, 1.0);
  if (randomSelector(generator) < exp((double)delta / currentTemp)){
    s = currentSol;
    //std::cout << "et acceptée";
  }
  //	std::cout << std::endl;
}


int SAA::displayCVS(const Problem& P, const Solution& s, int& solved){
  if (solved){
    std::cout << "1;";
    std::cout << s.getWeigthedObjectiveValue(P, alpha, beta) << ";"
	      << s.getSumCompletion(P) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(P) << ";"
	      << s.getNbSetup(P);
  }
  else
    std::cout << "0; ; ; ; ; ";
  return 0;
}

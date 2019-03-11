
#include "SimulatedAnnealing.h"



int SA(const Problem& problem, Solution& s, ptrHeur& computeSolInit){
  double tpsEcoule = 0.0;
  Clock::time_point t1 = Clock::now(); Clock::time_point t2;
  if (computeSolInit(problem, s)){
    int t = 0;
    double currentTemp = TEMP_INIT;
    std::sort(s.S.begin(), s.S.end());
    Solution currentSol(problem);
    Solution bestSol(s);
    int objBest = bestSol.getWeigthedObjectiveValue(problem);
    do{
      //std::cout << "temps �coul� : " << tpsEcoule << "\n temperature actuelle : " << currentTemp << "\n temperature change : " << t << std::endl;
      int cpt = 0;
      do{
	//std::cout << "\t generation of neighbor solution " << (t*NB_IT_PAR_TEMP) + cpt << std::endl;
	if (generateNeighborSol(problem, s, currentSol)){
	  //std::cout << "\t \t la solution est valide ";
	  int delta;
	  if ((delta = currentSol.getWeigthedObjectiveValue(problem) -
	       s.getWeigthedObjectiveValue(problem)) <= 0){
	    s = currentSol;
	    if (currentSol.getWeigthedObjectiveValue(problem) <
		objBest){
	      bestSol = currentSol;
	      objBest = bestSol.getWeigthedObjectiveValue(problem);
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

std::vector<std::vector<int>> computeLastOf(const Problem & problem, const Solution& s){
  const int F = problem.getNbFams();
  int j, f, i = 0;
  std::vector<std::vector<int>> lastOf(problem.M);

  for (j = 0; j < problem.M; ++j){
    lastOf[j].resize(F);
    for (f = 0; f < problem.getNbFams(); ++f)
      (problem.F[f].qualif[j] ? lastOf[j][f] = 0 : lastOf[j][f] = std::numeric_limits<int>::max());
  }
  while (i < problem.N){
    lastOf[s.S[i].machine][problem.famOf[s.S[i].index]] = s.S[i].start;
    ++i;
  }
  return lastOf;
}

int generateNeighborSol(const Problem& problem, const Solution &s, Solution& currentSol){
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> random(0, 100);
  currentSol = s;
  if (random(generator) < 50)
    return intraChange(problem, s, currentSol);
  else
    return interChange(problem, s, currentSol);
}

int intraChange(const Problem& problem, const Solution& s, Solution& currentSol){
  // tirage machine	
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomMachine(0, problem.M - 1);
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
    if (i < j) intraChangeMoveIBeforeJ(problem, s, currentSol, i, j, m);
    else if (i != j + 1) intraChangeMoveJBeforeI(problem, s, currentSol, i, j, m);

    //disqualification update
    currentSol.repairDisqualif(problem);
    return currentSol.isValid(problem);
  }
  else return 0;
}


void startTimeUpdate(const Problem& problem, const Solution& s, Solution& currentSol, int i, int j, int k){
  (problem.famOf[currentSol.S[k].index] == problem.famOf[s.S[j].index] ?
   currentSol.S[i].start = currentSol.S[k].start + problem.getDuration(currentSol.S[k].index) :
   currentSol.S[i].start = currentSol.S[k].start + problem.getDuration(currentSol.S[k].index) + problem.getSetup(s.S[j].index));
}

void intraChangeMoveIBeforeJ(const Problem& problem, const Solution& s, Solution& currentSol, int i, int j, int m){
  //move of j
  currentSol.S[i].index = s.S[j].index;
  if (i == 0 || s.S[i - 1].machine != m) currentSol.S[i].start = 0;
  else startTimeUpdate(problem, s, currentSol, i, j, i - 1);
  // move other until ex position of j
  for (int it = i + 1; it <= j; ++it){
    currentSol.S[it].index = s.S[it - 1].index;
    startTimeUpdate(problem, s, currentSol, it, it - 1, it - 1);
  }
  // move other after ex position of j
  for (int it = j + 1; it < problem.N && s.S[it].machine == m; ++it)
    startTimeUpdate(problem, s, currentSol, it, it, it - 1);
}

void intraChangeMoveJBeforeI(const Problem& problem, const Solution& s, Solution& currentSol, int i, int j, int m){
  // move between ex j and i
  currentSol.S[j].index = s.S[j + 1].index;
  if (j == 0 || s.S[j - 1].machine != m) currentSol.S[j].start = 0;
  else startTimeUpdate(problem, s, currentSol, j, j + 1, j - 1);
  for (int it = j + 1; it < i - 1; ++it){
    currentSol.S[it].index = s.S[it + 1].index;
    startTimeUpdate(problem, s, currentSol, it, it + 1, it - 1);
  }
  //move of j
  currentSol.S[i - 1].index = s.S[j].index;
  startTimeUpdate(problem, s, currentSol, i - 1, j, i - 2);
  // move other after ex position of j
  for (int it = i; s.S[it].machine == m && it < problem.N; ++it)
    startTimeUpdate(problem, s, currentSol, it, it, it - 1);
}

int interChange(const Problem& problem, const Solution& s, Solution& currentSol){
  // tirage machines
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> randomMachineI(0, problem.M - 1);
  std::uniform_int_distribution<int> randomMachineJ(0, problem.M - 2);
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
      interChangeUpdateMiThenMj(problem, s, currentSol, i, j, mi, mj);
    else
      interChangeUpdateMjThenMi(problem, s, currentSol, i, j, mi, mj);

    currentSol.repairDisqualif(problem);

  }
  return currentSol.isValid(problem);
}

void interChangeUpdateMjThenMi(const Problem& problem, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj){

  int it;
  currentSol.S[j].index = s.S[j + 1].index;
  currentSol.S[j].machine = s.S[j + 1].machine;
  if (j == 0 || s.S[j - 1].machine != s.S[j + 1].machine) currentSol.S[j].start = 0;
  else startTimeUpdate(problem, s, currentSol, j, j + 1, j - 1);
  for (it = j + 1; s.S[it + 1].machine == mj; ++it){
    currentSol.S[it].index = s.S[it + 1].index;
    startTimeUpdate(problem, s, currentSol, it, it + 1, it - 1);
  }
  for (it=it; it < i - 1; ++it)
    currentSol.S[it] = s.S[it + 1];
  //move de j
  currentSol.S[i - 1].index = s.S[j].index;
  currentSol.S[i - 1].machine = mi;
  if (currentSol.S[i - 2].machine != mi) currentSol.S[i - 1].start = 0;
  else startTimeUpdate(problem, s, currentSol, i - 1, j, i - 2);
  //update after j
  for (it = i; it < problem.N && s.S[it].machine == mi; ++it){
    currentSol.S[it].index = s.S[it].index;
    startTimeUpdate(problem, s, currentSol, it, it, it - 1);
  }
}
void interChangeUpdateMiThenMj(const Problem& problem, const Solution& s, Solution& currentSol, int i, int j, int mi, int mj){
  //move de j
  currentSol.S[i].index = s.S[j].index;
  if (i == 0 || s.S[i - 1].machine != mi) currentSol.S[i].start = 0;
  else startTimeUpdate(problem, s, currentSol, i, j, i - 1);
  //update after j
  int it;
  for (it = i + 1; s.S[it - 1].machine == mi; ++it){
    currentSol.S[it].index = s.S[it - 1].index;
    startTimeUpdate(problem, s, currentSol, it, it - 1, it - 1);
  }
  currentSol.S[it - 1].machine = mi;

  for (it =it; it <= j; ++it)
    currentSol.S[it] = s.S[it - 1];
  for (it = j + 1; it < problem.N && s.S[it].machine == mj; ++it){
    currentSol.S[it].index = s.S[it].index;
    if (s.S[it - 2].machine != mj) currentSol.S[it].start = 0;
    else startTimeUpdate(problem, s, currentSol, it, it, it - 1);
  }

}

void acceptWithProba(Solution& s, const Solution& currentSol, const double currentTemp, const int delta){

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<double> randomSelector(0.0, 1.0);
  if (randomSelector(generator) < exp((double)delta / currentTemp)){
    s = currentSol;
    //std::cout << "et accept�e";
  }
  //	std::cout << std::endl;
}


int SAA::displayCVS(const Problem& problem, const Solution& s, int& solved){
  if (solved){
    std::cout << "1;";
    std::cout << s.getWeigthedObjectiveValue(problem) << ";"
	      << s.getSumCompletion(problem) << ";"
	      << s.getNbDisqualif() << ";" << s.getRealNbDisqualif(problem) << ";"
	      << s.getNbSetup(problem);
  }
  else
    std::cout << "0; ; ; ; ; ";
  return 0;
}

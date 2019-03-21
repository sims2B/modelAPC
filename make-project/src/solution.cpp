#include "solution.h"

#include <algorithm>
#include <cassert>
#include <limits>

Solution::Solution(const Problem& p) : problem(p) {
  const int nbFam = problem.getNbFams();
  const int m = problem.getNbMchs();
  assign.resize(m);
  for (int i = 0; i < m; ++i) assign[i].reserve(problem.getNbJobs());

  qualifLostTime.resize(nbFam);
  for (int f = 0; f < nbFam; ++f)
    qualifLostTime[f].resize(problem.getNbMchs(),
                             std::numeric_limits<int>::max());
}

void Solution::clear() {
  const int m = problem.getNbMchs();
  for (int i = 0; i < m; ++i) assign[i].clear();
  assign.clear();
  assign.resize(m);
  for (int i = 0; i < m; ++i) assign[i].reserve(problem.getNbJobs());

  const int nbFam = problem.getNbFams();
  qualifLostTime.clear();
  qualifLostTime.resize(nbFam);
  for (int f = 0; f < nbFam; ++f)
    qualifLostTime[f].resize(problem.getNbMchs(),
                             std::numeric_limits<int>::max());
}

void Solution::addJob(const Job& j, int m) {
  std::vector<Job>::iterator it = assign[m].begin();

  while (it < assign[m].end() && (*it).getStart() < j.getStart()) ++it;

  if (it == assign[m].end())
    assign[m].push_back(j);
  else
    assign[m].insert(it, j);
}

void Solution::removeLastJob(int m) {
  assert(!assign[m].empty());
  assign[m].pop_back();
}

void Solution::removeJob(const Job& i, int m) {
  std::vector<Job>::iterator it = assign[m].begin();
  while (it != assign[m].end() && !(*it == i)) it++;
  assign[m].erase(it);
}

int Solution::getLastStart(int j) const { return getLastJob(j).getStart(); }

int Solution::getEnd(int j) const {
  if (getNbJobsOn(j) == 0) return 0;
  int idLast = assign[j].size() - 1;
  return assign[j][idLast].getStart() + problem.getDuration(assign[j][idLast]);
}

int Solution::getMaxEnd() const {
  int max = 0;
  for (unsigned int i = 0; i < assign.size(); ++i)
    (getEnd(i) > max ? max = getEnd(i) : max = max);
  return max;
}

int Solution::getSumCompletion() const {
  int sum = 0;
  for (unsigned int j = 0; j < assign.size(); ++j)
    for (uint i = 0; i < assign[j].size(); ++i)
      if (assign[j][i].getFam() != -1)
        sum += assign[j][i].getStart() + problem.getDuration(assign[j][i]);
  return sum;
}

Job Solution::getLastJob(int j) const {
  if (assign[j].empty())
    return Job(-1);
  else
    return assign[j][assign[j].size() - 1];
}

Job Solution::lastOf(int f, int m) const {
  Job j(f);
  for (uint i = 0; i < assign[m].size(); ++i)
    if (assign[m][i].getFam() == f) j = assign[m][i];
  return j;
}
Job Solution::firstOf(int f, int m) const {
  Job j(f);
  for (uint i = 0; i < assign[m].size(); ++i)
    if (assign[m][i].getFam() == f) {
      j = assign[m][i];
      break;
    }
  return j;
}

void Solution::getFirstOcc(Job& j, int f, int& m) const {
  int startMin = getMaxEnd();
  for (int k = 0; k < problem.getNbMchs(); ++k) {
    Job first = firstOf(f, k);
    if (first.getStart() != -1 && first.getStart() < startMin) {
        m = k;
        j = first;
        startMin = first.getStart();
    }
  }
}

Job Solution::getPreviousOcc(const Job& j, int f, int m) const {
  int i = assign[m].size();
  do {
    i--;
  } while (!(getJobs(i, m) == j));
  i--;
  while (i >= 0 && !(getJobs(i, m).getFam() == f)) i--;
  if (i < 0)
    return Job(f);
  else
    return getJobs(i, m);
}

Job Solution::nextOf(int i, int f, int m) const {
  do {
    i++;
  } while (i < (int)assign[m].size() && assign[m][i].getFam() != f);
  if (i < (int)assign[m].size())
    return assign[m][i];
  else
    return Job(f);
}

Job Solution::nextOf(const Job& j, int f, int m) const {
  int i = -1;
  do {
    i++;
  } while (!(assign[m][i] == j));
  i++;
  while (i < (int)assign[m].size() && assign[m][i].getFam() != f) ++i;
  if (i < (int)assign[m].size())
    return assign[m][i];
  else
    return Job(f);
}

void Solution::repairDisqualif() {
  const int F = problem.getNbFams();

  int Cmax = getMaxEnd();
  for (int f = 0; f < F; ++f)
    for (int j = 0; j < problem.getNbMchs(); ++j) {
      if (problem.isQualif(f, j)) {
        Job last = lastOf(f, j);
        if (last.getStart() == -1)
          qualifLostTime[f][j] = problem.getThreshold(f);
        else if (last.getStart() + problem.getThreshold(f) < Cmax)
          qualifLostTime[f][j] = last.getStart() + problem.getThreshold(f);
        else
          qualifLostTime[f][j] = std::numeric_limits<int>::max();
      }
    }
}

int Solution::getTotalNbDisqualif() const {
  int sum = 0;
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < std::numeric_limits<int>::max()) sum++;
  return sum;
}

int Solution::getNbQualif() const {
  int sum = 0;
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (problem.isQualif(f, j) &&
          qualifLostTime[f][j] >= std::numeric_limits<int>::max())
        sum++;
  return sum;
}

int Solution::getNbDisqualif() const {
  int sum = 0;
  int Cmax = getMaxEnd();
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f)
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < Cmax) sum++;
  return sum;
}

int Solution::getWeigthedObjectiveValue() const {
  return alpha_C * getSumCompletion() + beta_Y * getNbDisqualif();
}

int Solution::getNbSetup(int m) const {
  int nbSet = 0;
  for (uint i = 0; i < assign[m].size() - 1; ++i)
    if (assign[m][i].getFam() != assign[m][i + 1].getFam()) nbSet++;
  return nbSet;
}

int Solution::getNbSetup() const {
  int nbSet = 0;
  for (uint m = 0; m < assign.size(); ++m) nbSet += getNbSetup(m);
  return nbSet;
}

int Solution::getNbJobsOn(int m) const { return assign[m].size(); }

Job Solution::getJobs(int i, int m) const { return assign[m][i]; }

void Solution::shiftJob(int i, int m, int shift) {
  assign[m][i].shiftStart(shift);
}

int Solution::isValid() const {
  int i, j;
  std::vector<int> toDo(problem.getNbFams(), 0);

  // all task executed
  for (j = 0; j < problem.getNbMchs(); ++j)
    for (i = 0; i < getNbJobsOn(j); ++i)
      if (assign[j][i].getFam() != -1) toDo[assign[j][i].getFam()]++;
  for (uint k = 0; k < toDo.size(); ++k)
    if (toDo[k] != problem.getNf(k)) return 0;
  //  std::cout << "all task executed\n";

  // overlap
  for (j = 0; j < problem.getNbMchs(); ++j)
    if (getNbJobsOn(j) != 0)
      for (i = 0; i < getNbJobsOn(j) - 1; i++)
        if (assign[j][i].getStart() + problem.getDuration(assign[j][i]) >
            assign[j][i + 1].getStart())
          return 0;

  // std::cout << "overlap ok\n";
  // setup
  for (j = 0; j < problem.getNbMchs(); ++j)
    if (getNbJobsOn(j) != 0)
      for (i = 0; i < getNbJobsOn(j) - 1; ++i) {
        int setup = 0;
        if (assign[j][i].getFam() != assign[j][i + 1].getFam())
          setup = problem.getSetup(assign[j][i + 1]);
        if (assign[j][i].getStart() + problem.getDuration(assign[j][i]) +
                setup >
            assign[j][i + 1].getStart())
          return 0;
      }
  // std::cout << "setup ok\n";

  // when the task is processed, the machine is still
  // qualified

  for (j = 0; j < problem.getNbMchs(); ++j)
    if (getNbJobsOn(j) != 0)
      for (i = 0; i < getNbJobsOn(j) - 1; ++i) {
        if (assign[j][i].getStart() + problem.getDuration(assign[j][i]) >
            qualifLostTime[assign[j][i].getFam()][j])
          return 0;
      }
  // std::cout << "when the task is processed, the machine is still
  // qualified\n";

  // the machine j becomes disqualified for f if there is no
  // task of f in an interval gamma_f;
  const int Cmax = getMaxEnd();

  for (j = 0; j < problem.getNbMchs(); ++j)
    for (int f = 0; f < problem.getNbFams(); ++f)
      if (problem.isQualif(f, j))
        for (int t = problem.getThreshold(f); t < Cmax; ++t) {
          i = 0;
          while (i < getNbJobsOn(j) &&
                 !(assign[j][i].getFam() == f &&
                   assign[j][i].getStart() > t - problem.getThreshold(f) &&
                   assign[j][i].getStart() <= t))
            ++i;
          if (i >= getNbJobsOn(j) && qualifLostTime[f][j] > t) {
            std::cout << " pb sur f = " << f << "on " << j << "at time " << t
                      << std::endl;
            return 0;
          }
        }
  // std::cout << "the machine j becomes disqualified for f if there is no  task
  // "
  //             "of f in an interval gamma_f\n";

  // no more than gamma_f between to task of the same
  // family;
  for (j = 0; j < problem.getNbMchs(); ++j) {
    for (i = 0; i < getNbJobsOn(j); ++i) {
      Job next = nextOf(i, assign[j][i].getFam(), j);
      if (next.getStart() != -1) {
        if (next.getStart() - assign[j][i].getStart() >
            problem.getThreshold(assign[j][i]))
          return 0;
      }
    }
  }
  // std::cout << "no more than gamma_f between to task of the same family\n ";
  return 1;
}

std::string Solution::toString() const {
  std::string res =
      "Une solution est : \n - les dates de début des tâches sont: "
      "\n";
  for (uint j = 0; j < assign.size(); ++j) {
    res += " sur la machine " + std::to_string(j) + ": \n";
    for (uint i = 0; i < assign[j].size(); ++i) {
      res += assign[j][i].toString();
    }
  }
  res += " - Les familles ayant perdues leur qualification sont :\n";
  for (unsigned int f = 0; f < qualifLostTime.size(); ++f) {
    res += " * la famille " + std::to_string(f) +
           " a perdu sa qualif sur les machines:\n ";
    for (unsigned int j = 0; j < qualifLostTime[f].size(); ++j)
      if (qualifLostTime[f][j] < std::numeric_limits<int>::max())
        res += "\t - " + std::to_string(j) + " au temps " +
               std::to_string(qualifLostTime[f][j]) + "\n";
    res += "\n";
  }

  res += "Le nombre de Machines disqualifiées est : " +
         std::to_string(getNbDisqualif()) +
         " et la sum des completion times vaut :" +
         std::to_string(getSumCompletion()) + "\n";
  return res;
}

void Solution::toTikz() const {
  std::cout << "\\begin{tikzpicture}\n"
            << "\\node (O) at (0,0) {};\n"
            << "\\draw[->] (O.center) -- ( " << getMaxEnd() + 1 << ",0);\n"
            << "\\draw[->] (O.center) -- (0, "
            << (problem.getNbMchs() * 0.5 + 0.5) << ");\n";
  for (int t = 5; t <= getMaxEnd(); t = t + 5)
    std::cout << "\\draw (" << t << ",0) -- (" << t << ",-0.1) node[below] {$"
              << t << "$} ;\n";
  for (uint j = 0; j < assign.size(); ++j) {
    for (uint i = 0; i < assign[j].size(); ++i) {
      std::cout << "\\draw[fill = " << tikzColor[assign[j][i].getFam()]
                << "!80!black!80]  (" << assign[j][i].getStart() << ","
                << ((int)j * 0.5) << ") rectangle ("
                << assign[j][i].getStart() + problem.getDuration(assign[j][i])
                << "," << ((int)j * 0.5 + 0.5) << ") node[midway] {$"
                << assign[j][i].getFam() << "$};\n";
    }
  }
  std::cout << "\\end{tikzpicture}\n";
}

void Solution::toDimacs() const {
  std::cout << "d CMAX " << getMaxEnd() << std::endl
            << "d FLOWTIME " << getSumCompletion() << std::endl
            << "d DISQUALIFIED " << getNbDisqualif() << std::endl
            << "d QUALIFIED " << getNbQualif() << std::endl
            << "d SETUP " << getNbSetup() << std::endl
            << "d VALIDE " << isValid() << std::endl;
}

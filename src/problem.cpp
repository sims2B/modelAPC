#include "problem.h"
#include <algorithm>
#include <cassert>
#include <random>

Job::Job(int _f) : fam(_f), start(-1), index(-1) {}

Job::Job(int _f, int st, int id) : fam(_f), start(st), index(id) {}

std::string Job::toString() const {
  std::string res;
  res += "  \t * une tache de la famille " + std::to_string(getFam()) +
         " commence au temps " + std::to_string(getStart()) + " \n";
  return res;
}

Job& Job::operator=(const Job& j) {
  if (&j != this) {
    start = j.start;
    index = j.index;
    (int&)fam = j.fam;
  }
  return *this;
}

bool operator==(const Job& i, const Job& j) {
  return (i.getFam() == j.getFam() && i.getIndex() == j.getIndex() &&
          i.getStart() == j.getStart());
}

bool operator<(const Job& i, const Job& j) {
  return (i.getFam() < j.getFam() || ((i.getFam() == j.getFam()) && i.getStart() < j.getStart()));
}

Family::Family(int m) : duration(0), setup(0), threshold(0), nbJobs(0) {
  qualif.resize(m, 0);
}

Family::Family(int _dur, int _setup, int _threshold, int nf, int m)
    : duration(_dur), setup(_setup), threshold(_threshold), nbJobs(nf) {
  qualif.resize(m, 0);
}

Family::Family(int _dur, int _setup, int _threshold, int nf,
               std::vector<int> _qualif)
    : duration(_dur),
      setup(_setup),
      threshold(_threshold),
      nbJobs(nf),
      qualif(_qualif) {}

int Family::writeFamily(std::ofstream& out) const {
  out << duration << " " << threshold << " " << setup << " " << nbJobs << " "
      << qualif[0];
  for (unsigned int i = 1; i < qualif.size(); ++i) out << " " << qualif[i];
  out << std::endl;
  return 0;
}

std::string Family::toString() const {
  std::string res = "La famille comporte " + std::to_string(nbJobs) +
                    " jobs et a une durée de " + std::to_string(duration) +
                    ", un setup time de " + std::to_string(setup) +
                    ", un threshold de " + std::to_string(threshold) +
                    " et les machines ";
  for (unsigned int i = 0; i < qualif.size(); ++i) {
    if (qualif[i]) res += std::to_string(i) + " ";
  }
  res += " sont qualifiées pour son execution\n";
  return res;
}

Family readFamily(std::ifstream& in, int M) {
  int d, s, t, nf;
  std::vector<int> qualif;
  in >> d >> t >> s >> nf;
  for (int i = 0; i < M; ++i) {
    int q;
    in >> q;
    qualif[i] = q;
  }
  return Family(d, s, t, nf, qualif);
}

Family oldReader(std::ifstream& in, int M, int size) {
  int d, s, t;
 
  std::vector<int> qualif(M,0);
  in >> d >> t >> s ;
 
  for (int i = 0; i < M; ++i) {
    int q;
    in >> q;
    qualif[i] = q;
  }
  return Family(d, s, t, size, qualif);
}

Problem::Problem(int nbTask, int nbFam) : N(nbTask), M(0) { F.reserve(nbFam); }

Problem::Problem(int nbTask, int nbMach, int nbFam) : N(nbTask), M(nbMach) {
  Family _F(M);
  F.resize(nbFam, _F);
}

Problem::Problem(const Problem& p) {
  N = p.getNbJobs();
  M = p.getNbMchs();
  F = p.getFamList();
}

Problem::Problem(int nbTask, int nbMach, familyList _F)
    : N(nbTask), M(nbMach), F(_F) {}

int Problem::writeInFile(std::ofstream& out) const {
  unsigned int i;
  out << N << " " << M << " " << getNbFams() << std::endl;
  for (i = 0; i < F.size(); ++i) F[i].writeFamily(out);
  return 0;
}

int Problem::computeHorizon() const {
  int res = 0;
  for (int f = 0; f < getNbFams(); ++f)
    res += F[f].getNbJobs() * (getDuration(f) + getSetup(f));
  return res;
}

int Problem::getFamily(int i) const {
  int deb, fin = 0;
  for (int f = 0; f < getNbFams(); ++f) {
    deb = fin;
    fin = deb + F[f].getNbJobs();
    if (i < fin && i >= deb) return f;
  }
  return -1;
}

std::string Problem::toString() const {
  std::string res =
      "Le problème possède les caractéristiques suivantes:\n - le nombre de "
      "tâches est :" +
      std::to_string(N) +
      "\n - le nombre de machine est :" + std::to_string(M) + "\n - \n";
  res += "- Et les familles (" + std::to_string(F.size()) +
         ") possèdent les caractéristiques suivantes:\n";
  for (unsigned int f = 0; f < F.size(); ++f)
    res += "   * " + std::to_string(f) + ": " + F[f].toString();
  return res;
}

void Problem::toDimacs() const {
  std::cout << "c JOBS " << N << std::endl
            << "c MACHINES " << M << std::endl
            << "c FAMILIES " << getNbFams() << std::endl;
}

// reader
Problem readFromFile(std::ifstream& in) {
  unsigned int i, nbF, N, M;
  familyList F;
  F.reserve(nbF);

  in >> N >> M >> nbF;
  for (i = 0; i < nbF; ++i) F.push_back(readFamily(in, M));
  return Problem(N, M, F);
}
//reader
Problem oldReader(std::ifstream& in){
  unsigned int i , nbF , N, M;
  int temp;
  familyList F;

  in >> N >> M >> nbF;
  F.reserve(nbF);

  std::vector<int> famSize(nbF,0);
  for (i = 0 ; i < N ; ++i){
    in >> temp;
    famSize[temp]++;
  }

  for (i = 0; i < nbF; ++i) F.push_back(oldReader(in, M, famSize[i] ));
  return Problem(N, M, F);
}
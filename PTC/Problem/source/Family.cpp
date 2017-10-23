#include "Family.h"
#include <vector>
Family::Family(int m) : duration(0), setup(0), threshold(0) {
  qualif.clear();qualif.resize(m,0);
}

Family::Family(int _dur,int _setup, int _threshold,  int m):
  duration(_dur), setup(_setup), threshold(_threshold){
  qualif.resize(m,0);
}

Family::Family(int _dur,int _setup, int _threshold, std::vector<int> _qualif):
  duration(_dur), setup(_setup), threshold(_threshold), qualif(_qualif){}


std::string Family::toString() const{
  std::string res = "La famille a une durée de " + std::to_string(duration) + ", un setup time de " + std::to_string(setup) + ", un threshold de " +std::to_string(threshold) + " et les machines ";
  for (uint i = 0 ; i < qualif.size() ; ++i){
    if (qualif[i])
      res+= std::to_string(i) + " " ;
  }
  res += " sont qualifiées pour son execution\n";
  return res;
}

#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#define POSITIVE_ZERO 0.00001
#define NEGATIVE_ZERO -0.00001

void printVector(std::string name , std::vector<int> v);
std::string getBasename(const std::string& str);
inline int isNul(const double& num){
return (num <= POSITIVE_ZERO && num >= NEGATIVE_ZERO);
}

/*inline std::string to_string(int const val) {
std::stringstream ss;
ss << val;
std::string numberAsString(ss.str());
    return  numberAsString;
}*/


#include "utils.h"

std::string getFilename(std::string filePath, bool withExtension, char seperator)
{
  std::size_t sepPos = filePath.rfind(seperator);
  std::size_t dotPos = filePath.rfind('.');
  std::size_t s = sepPos == std::string::npos ? 0 : sepPos + 1;
  std::size_t e = (withExtension || dotPos == std::string::npos) ? filePath.size() : dotPos;
  // std::cout << filePath <<  " " <<  withExtension << " "<<s << " " << e << std::endl;
  return filePath.substr(s, e-s);
}



#include <iostream>
#include <vector>
#include <sstream>
#include <string>


/*
 * Get File Name from a Path with or without extension
 * From https://thispointer.com/c-how-to-get-filename-from-a-path-with-or-without-extension-boost-c17-filesytem-library/
 */
std::string getFilename(std::string filePath, bool withExtension = true, char seperator = '/');
/* { */
/* 	// Get last dot position */
/* 	std::size_t dotPos = filePath.rfind('.'); */
/* 	std::size_t sepPos = filePath.rfind(seperator); */
 
/* 	if(sepPos != std::string::npos) */
/* 	{ */
/* 		return filePath.substr(sepPos + 1, filePath.size() - (withExtension || dotPos != std::string::npos ? 1 : dotPos) ); */
/* 	} */
/* 	return ""; */
/* } */
  
  

/*inline std::string to_string(int const val) {
std::stringstream ss;
ss << val;
std::string numberAsString(ss.str());
    return  numberAsString;
}*/


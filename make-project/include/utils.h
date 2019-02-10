#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

/*
 * Get File Name from a Path with or without extension
 * From https://thispointer.com/c-how-to-get-filename-from-a-path-with-or-without-extension-boost-c17-filesytem-library/
 */
std::string getFilename(std::string filePath, bool withExtension = true, char seperator = '/');
  
#endif

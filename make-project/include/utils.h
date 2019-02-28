#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
// CPU Time
// #include <ctime>
// Wall Clock Time
#include <time.h>

/*
 * Get File Name from a Path with or without extension
 * From https://thispointer.com/c-how-to-get-filename-from-a-path-with-or-without-extension-boost-c17-filesytem-library/
 */
std::string getFilename(std::string filePath, bool withExtension = true, char seperator = '/');

typedef struct TimeStamp { 
    std::string label;
    double elapsedTime; 
} TimeStamp;

class Timer {
    private:
  time_t startTime;
  time_t stageTime;
  std::vector<TimeStamp> timestamps; 

  public:
  
  void start() {
    startTime = time(NULL);
    stageTime = startTime;  
  };

  void stage() {
    stageTime = time(NULL);
  }  
  
  void stage(std::string label) {
    time_t currentTime = time(NULL);
    double elapsed = difftime(currentTime, stageTime);
    TimeStamp stamp = {label, elapsed};
    timestamps.push_back(stamp);
    stageTime = currentTime;
  };

  void stop(std::string label = "WCTIME") {
    time_t currentTime = time(NULL);
    double elapsed = difftime(currentTime, startTime);
    TimeStamp stamp = {label, elapsed};
    timestamps.push_back(stamp);
  };

  void toDimacs() {
     for(auto& stamp : timestamps) { 
        printf("d %s %.3f\n", stamp.label.c_str(), stamp.elapsedTime);
     }
  };
  
};


#endif

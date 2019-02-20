#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <ctime>

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
  std::clock_t startTime;
  std::clock_t stageTime;
  std::vector<TimeStamp> timestamps; 

  public:
  
  void start() {
    startTime = std::clock();
    stageTime = startTime;  
  };

  void stage() {
    stageTime = std::clock();
  }  
  
  void stage(std::string label) {
    std::clock_t currentTime = std::clock();
    double elapsed = ((double) (currentTime - stageTime)) / CLOCKS_PER_SEC;
    TimeStamp stamp = {label, elapsed};
    timestamps.push_back(stamp);
    stageTime = currentTime;
  };

  void stop() {
    std::clock_t stageTime = std::clock();
    double elapsed = ((double) (stageTime - startTime))  / CLOCKS_PER_SEC;
    TimeStamp stamp = {"WCTIME", elapsed};
    timestamps.push_back(stamp);
  };

  void toDimacs() {
     for(auto& stamp : timestamps) { 
        printf("d %s %.3f\n", stamp.label.c_str(), stamp.elapsedTime);
     }
  };
  
};

#endif

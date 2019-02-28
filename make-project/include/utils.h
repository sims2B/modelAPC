#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
/*
 * Get File Name from a Path with or without extension
 * From https://thispointer.com/c-how-to-get-filename-from-a-path-with-or-without-extension-boost-c17-filesytem-library/
 */
std::string getFilename(std::string filePath, bool withExtension = true, char seperator = '/');

typedef struct TimeStamp
{
  std::string label;
  double elapsedTime;
} TimeStamp;

class Timer
{
  // alias our types for simplicity
  using clock = std::chrono::system_clock;
  using time_point_type = std::chrono::time_point<clock>;

private:
  time_point_type startTime;
  time_point_type stageTime;
  std::vector<TimeStamp> timestamps;

  double difftime(time_point_type t1, time_point_type t0)
  {
    return ((double)(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count())) / 1000;
  }

  void pushtime(std::string label, time_point_type t1, time_point_type t0)
  {
    TimeStamp stamp = {label, difftime(t1, t0)};
    timestamps.push_back(stamp);
  }

public:
  Timer() {
    startTime = clock::now();
    stageTime = startTime;
  }

  void stage()
  {
    stageTime = clock::now();
  }

  void stage(std::string label)
  {
    time_point_type currentTime = clock::now();
    pushtime(label, currentTime, stageTime);
    stageTime = currentTime;
  };

  void stagewc(std::string label = "WCTIME")
  {
    time_point_type currentTime = clock::now();
    pushtime(label, currentTime, startTime);
    stageTime = currentTime;
  };

  void toDimacs() const
  {
    for (auto &stamp : timestamps)
    {
      printf("d %s %.3f\n", stamp.label.c_str(), stamp.elapsedTime);
    }
  };
};

#endif

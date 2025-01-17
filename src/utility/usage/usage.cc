#include <string.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "usage.hh"

namespace ieda {

Stats::Stats() {
  _memory_begin = memoryUsage();
  getTimeOfDay(&_elapsed_begin_time);
}

/**
 * @brief return peak virtual memory usage in kilobytes now
 *
 * @return Peak virtual memory (VmPeak) size of current process now
 * @note rusage->ru_maxrss is not set in linux so read it from /proc
 * @see Linux Programmer's Manual PROC(5)
 */
size_t Stats::memoryUsage() const {
  std::ostringstream buf("/proc/", std::ios_base::ate);
  buf << getpid();
  buf << "/status";

  std::string proc_filename = buf.str();

  size_t memory = 0;
  FILE *status = fopen(proc_filename.c_str(), "r");
  if (status) {
    const size_t line_length = 128;
    char line[line_length];
    char *field;
    char *saveptr;

    while (fgets(line, line_length, status) != nullptr) {
      field = strtok_r(line, " \t", &saveptr);
      if (!strcmp(field, "VmPeak:")) {
        char *size = strtok_r(saveptr, " \t", &saveptr);
        if (size) {
          char *ignore;
          // VmPeak is in kilobytes.
          memory = strtol(size, &ignore, 10) * 1000;
          break;
        }
      }
    }
    fclose(status);
  }
  return memory;
}

/**
 * @brief get the program use memory.
 *
 * @return double
 */
double Stats::memoryDelta() const {
  double memory_end = static_cast<double>(memoryUsage());
  double memory_delta = memory_end - _memory_begin;
  return memory_delta * 1e-6;
}

/**
 * @brief get the current wall time.
 *
 * @return std::string
 */
std::string Stats::getCurrentWallTime() const {
  time_t timer;
  struct tm *tblock;
  timer = time(nullptr);
  tblock = localtime(&timer);

  return asctime(tblock);
}

/**
 * @brief get the time of day.
 *
 * @param tv
 * @return int
 */
int Stats::getTimeOfDay(struct timeval *tv) const {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  tv->tv_sec = ts.tv_sec;
  tv->tv_usec = ts.tv_nsec / 1000;
  return 0;
}

/**
 * @brief get the program use time.
 *
 * @return double
 */
double Stats::elapsedRunTime() const {
  static struct timeval time;

  getTimeOfDay(&time);
  return time.tv_sec - _elapsed_begin_time.tv_sec +
         (time.tv_usec - _elapsed_begin_time.tv_usec) * 1e-6;
}

}  // namespace ieda
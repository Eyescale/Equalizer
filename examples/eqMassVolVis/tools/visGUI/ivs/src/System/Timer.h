/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_TIMER_H
#define IVS_SYSTEM_TIMER_H

#include "System/Config.h"

#ifdef IVS_WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

namespace ivs
{
namespace sys
{

#ifdef IVS_WIN32
int gettimeofday(struct timeval *tv, int);
#endif

/**
 * A simple timer
 **/
class Timer
{
public:
  Timer();
  ~Timer();

  void start();
  void stop();

  double getElapsedTime() const;
  void printElapsedTime() const;

private:
  timeval startTime_, endTime_;
};

inline void Timer::start()
{
  gettimeofday(&startTime_, 0);
}

inline void Timer::stop()
{
  gettimeofday(&endTime_, 0);
}

}
}

#endif

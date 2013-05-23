/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "System/Timer.h"

namespace ivs
{
namespace sys
{

#ifdef IVS_WIN32
int gettimeofday(struct timeval *tv, int)
{
  union
  {
    long long ns100;
    FILETIME ft;
  } now;

  GetSystemTimeAsFileTime(&now.ft);
  tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
  tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL);
  return (0);
}
#endif

Timer::Timer()
{}

Timer::~Timer()
{}

double Timer::getElapsedTime() const
{
  double elapsed = (endTime_.tv_sec - startTime_.tv_sec) +
                   (endTime_.tv_usec - startTime_.tv_usec) / 1000000.0;
  return elapsed;
}

}
}

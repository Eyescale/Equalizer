/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "System/Logger.h"

#ifdef CURSES
#  include <cstdlib>
#endif
#include <sstream>
#include <iostream>

#include <boost/date_time.hpp>

namespace ivs
{
namespace sys
{
using namespace boost::posix_time;

Logger                *Logger::instance_ = 0;
Destroyer<Logger>      Logger::destroyer_;
#ifdef CURSES
WINDOW                *CursesLogger::window_ = 0;
#endif

std::string Logger::getErrorMessage(const std::string &_message, const char *_file, int _line)
{
  std::stringstream s;
  std::string f(_file);
  size_t i = f.find_last_of("/\\");
  s << f.substr(i + 1) << "." << _line << ": " << _message;
  return s.str();
}

Logger *Logger::instance()
{
  if (!instance_)
  {
#ifdef CURSES
    char *logger = getenv("IVS_LOGGER");
    if (logger != 0 && strcmp(logger, "CURSES") == 0)
      instance_ = new CursesLogger();
    else
#endif
      instance_ = new CoutLogger();
    destroyer_.setSingleton(instance_);
  }
  return instance_;
}

Logger::Logger()
{}

Logger::~Logger()
{
  instance_ = 0;
}

void Logger::logWithDateTime(const std::string &_message)
{
  std::stringstream ss;
  const ptime now = second_clock::local_time();
  ss.imbue(std::locale(ss.getloc(), new time_facet("%d.%m.%Y %H:%M:%S")));
  ss << now << ": " << _message;
  log(ss.str());
}

CoutLogger::CoutLogger():
  Logger()
{}

CoutLogger::~CoutLogger()
{}

#ifdef CURSES
CursesLogger::CursesLogger():
  Logger()
{
  if (window_ == 0)
    window_ = initscr();
}

CursesLogger::~CursesLogger()
{
  endwin();
}
#endif

}
}

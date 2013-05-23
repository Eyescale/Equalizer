/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_LOGGER_H
#define IVS_SYSTEM_LOGGER_H

#include <string>
#include <iostream>
#ifdef CURSES
#  include <curses.h>
#endif

#include "System/Destroyer.h"

#define GET_ERROR_MESSAGE(MESSAGE) ivs::sys::Logger::getErrorMessage(MESSAGE, __FILE__, __LINE__)

namespace ivs
{
namespace sys
{

/**
 * Message logger interface.
 * Patterns: Singleton
 **/
class Logger
{
public:
  static std::string getErrorMessage(const std::string &_message, const char *_file, int _line);
  static Logger *instance();

  virtual void log(const std::string &_message) = 0;
  virtual void logWithDateTime(const std::string &_message);

protected:
  friend class Destroyer<Logger>;

  Logger();
  virtual ~Logger();

  static Logger            *instance_;
  static Destroyer<Logger>  destroyer_;
};

/**
 * Basic logger using std::cout
 **/
class CoutLogger: public Logger
{
public:
  virtual void log(const std::string &_message);

protected:
  friend class Destroyer<Logger>;
  friend class Logger;

  CoutLogger();
  virtual ~CoutLogger();
};

#ifdef CURSES
/**
 * Basic logger using curses
 **/
class CursesLogger: public Logger
{
public:
  virtual void log(const std::string &_message);

  static WINDOW *getWindow();

  static void refresh();

protected:
  friend class Destroyer<Logger>;
  friend class Logger;

  CursesLogger();
  virtual ~CursesLogger();

  static WINDOW *window_;
};
#endif

inline void CoutLogger::log(const std::string &_message)
{
  std::cout << _message;
}

#ifdef CURSES
inline WINDOW *CursesLogger::getWindow()
{
  return window_;
}

inline void CursesLogger::refresh()
{
  if (window_ != 0)
    wrefresh(window_);
}

inline void CursesLogger::log(const std::string &_message)
{
  mvwprintw(window_, 30, 0, "%s", _message.c_str());
}
#endif

}
}

#endif

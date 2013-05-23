
#ifndef UTIL_STAT_LOGGER_H
#define UTIL_STAT_LOGGER_H

#include <msv/types/nonCopyable.h>

#include <string>
#include <sstream>
#include <list>

#include <iostream>
#include <stdlib.h>

namespace lunchbox{ class Clock; }

namespace util
{
class StatLogger;

/**
 * Logger for statistics. Different threads/classes can log various events.
 * Name of the logging instance, time and message will be saved. Messages
 * can contain multiple values and are separated by std::endl.
 *
 * Use StatLogger class to allocate loaders when aggreagate statistics is
 * required.
 */
class EventLogger: private massVolVis::NonCopyable
{
public:
    /**
     * Used to log various data. Time of a log is the time when first value is
     * passed, log is saved whenever std::endl appears.
     */
    template<class T> EventLogger& operator << ( const T& value );

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType; // type of std::cout
    typedef CoutType& (*StandardEndLine)(CoutType&); // function signature for std::endl

    EventLogger& operator<<( StandardEndLine manip ); // detects std::endl

    double getTimed() const;
private:
    friend class StatLogger;

    EventLogger( const std::string& name, const StatLogger* owner );

    /**
     * Sets time of current event. Called from "operator <<" if new 
     * message is being logged.
     */
    void _storeEventTime();

    std::string _name;     // name of current logger
    std::stringstream _ss; // last entered string
    double      _time;     // time of the event

    struct Entity
    {
        Entity( double time_, const std::string& msg_ ) : time( time_ ), msg( msg_ ){}
        double      time; // time of the log
        std::string msg;  // log message itself
    };
    std::list< Entity > _messages;   // messages of current logger
          EventLogger*  _next;       // next logger if loggers are arranged in a list
    const StatLogger*   _statLogger; // owner of the class, used to get timer
};

/**
 * Dispatcher for multiple statistics loggers. Allocates loaders and
 * mixes final statistics from different logging instances together.
 */
class StatLogger : private massVolVis::NonCopyable
{
public:
    static StatLogger& instance()
    {
        static StatLogger logger;
        return logger;
    }

    /**
     * Creates and returns new logger instance. Name parameter is 
     * not checked for duplicates.
     * 
     * @param name name of a new logger
     */
    EventLogger* createLogger( const std::string& name );

    const lunchbox::Clock* getTimer() const { return _timer; }

    void printStats();

    void setLogFilePath( const std::string& path ) { _logFilePath = path; }
private:
    StatLogger();
    ~StatLogger();

    EventLogger* _loggersFirst;
    EventLogger* _loggersLast;
    lunchbox::Clock* _timer;
    std::string _logFilePath;
};



template<>
EventLogger& EventLogger::operator<< <std::string>( const std::string& value );


template<class T>
EventLogger& EventLogger::operator<< ( const T& value )
{
    if( _ss.tellg() == 0 )
        _storeEventTime();
    _ss << value;
    return *this;
}


}

#endif // UTIL_STAT_LOGGER_H

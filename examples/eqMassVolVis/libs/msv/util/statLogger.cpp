
#include "statLogger.h"

#include <lunchbox/lock.h>
#include <lunchbox/clock.h>
#include <lunchbox/debug.h>

#include <iostream>
#include <fstream>

namespace util
{

EventLogger::EventLogger( const std::string& name, const StatLogger* owner )
    : _name( name )
    , _next( 0 )
    , _statLogger( owner )
{
    LBASSERT( _statLogger );
}

EventLogger& EventLogger::operator<<( StandardEndLine manip )
{
    _messages.push_back( Entity( _time, _ss.str() ));
    _ss.str( "" );
    _ss.clear();
    return *this;
}

double EventLogger::getTimed() const
{
    return _statLogger->getTimer()->getTimed();
}

void EventLogger::_storeEventTime()
{
    _time = getTimed();
}

template<>
EventLogger& EventLogger::operator<< <std::string>( const std::string& value )
{
    _ss << value.c_str();
    return *this;
}


// ================== StatLogger functions =====================

StatLogger::StatLogger()
    : _loggersFirst( 0 )
    , _loggersLast(  0 )
    , _timer( new lunchbox::Clock() )
{
    _timer->reset();
    std::cout << "StatLogger constructor" << std::endl;
}


StatLogger::~StatLogger()
{
    std::cout << "StatLogger destructor" << std::endl;

    while( _loggersFirst != 0 )
    {
        EventLogger* current = _loggersFirst;
        _loggersFirst = _loggersFirst->_next;
        delete current;
    }
    _loggersLast = 0;

    delete _timer; _timer = 0;
}

void StatLogger::printStats()
{
/*    EventLogger* logger = _loggersFirst;
    while( logger != 0 )
    {
        std::cout << "name: " << logger->_name.c_str() << std::endl;

        std::list< EventLogger::Entity >::iterator msgIt = logger->_messages.begin();
        while( msgIt != logger->_messages.end() )
        {
            const EventLogger::Entity& msg = *msgIt;
            std::cout << "name: " << logger->_name.c_str() << " time: " << msg.time << ", str: " << msg.msg << std::endl;
            msgIt++;
        }
        logger = logger->_next;
    }
*/
    std::ofstream ofs;
    if( !_logFilePath.empty() )
    {
        ofs.open( _logFilePath.c_str(), std::ios_base::out | std::ios_base::trunc  );
        if( !ofs.is_open( ))
            LBERROR << "Can't open file to write: " << _logFilePath.c_str() << std::endl;
    }

    std::ostream& os = ofs.is_open() ? ofs : std::cout;

    while( true )
    {
        EventLogger* firstLogger = _loggersFirst;
        while( firstLogger && firstLogger->_messages.size() == 0 )
            firstLogger = firstLogger->_next;

        if( !firstLogger )
            break;

        EventLogger* minLogger = firstLogger;
        EventLogger* nextLogger = minLogger->_next;
        while( nextLogger )
        {
            if( nextLogger->_messages.size() > 0 &&
                nextLogger->_messages.front().time < minLogger->_messages.front().time )
                minLogger = nextLogger;
            nextLogger = nextLogger->_next;
        }
        EventLogger::Entity msg = minLogger->_messages.front();
        minLogger->_messages.pop_front();
        os << "name: " << minLogger->_name.c_str() << " time: " << msg.time << ", str: " << msg.msg << std::endl;
    }
}


EventLogger* StatLogger::createLogger( const std::string& name )
{
    static lunchbox::Lock lock;

    EventLogger* slg = new EventLogger( name, this );
    lock.set();
    if( _loggersLast )
    {
        _loggersLast->_next = slg;
        _loggersLast = slg;
    }else
    {
        LBASSERT( _loggersFirst == 0 );
        _loggersFirst = slg;
        _loggersLast  = slg;
    }
    lock.unset();
    return slg;
}

}

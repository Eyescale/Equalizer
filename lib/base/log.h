
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <iostream>
#include <pthread.h>
#include <sstream>
#include <time.h>
#include <unistd.h>

#ifndef NDEBUG
#  include "clock.h"
#endif

/**
 * @namespace eqBase
 * @brief Namespace for basic Equalizer utility code.
 */
namespace eqBase
{
    /** The logging levels. */
    enum LogLevel
    {
        LOG_ERROR,
        LOG_WARN,
        LOG_INFO,
        LOG_VERBATIM
    };

    /** The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
	public:
		LogBuffer( std::ostream& stream )
                : _indent(0), _newLine(true), _stream(stream)
            {}
        
        void indent(){ ++_indent; }
        void exdent(){ --_indent; }
        
	protected:
        virtual int_type overflow (int_type c) 
            {
                if( c == EOF )
                    return EOF;

                if( _newLine )
                {
                    for( int i=0; i<_indent; ++i )
                        _stringStream << "    ";
                    _newLine = false;
                }

                _stringStream << (char)c;
                return c;
            }
        
        virtual int sync() 
            {
                const std::string& string = _stringStream.str();
                _stream.write( string.c_str(), string.length( ));
                _stringStream.str("");
                _newLine = true;
                return 0;
            }

    private:
        LogBuffer( const LogBuffer& );
        LogBuffer& operator = ( const LogBuffer& );

        /** The current indentation level. */
        int _indent;

        /** The flag that a new line has started. */
        bool _newLine;

        /** The temporary buffer. */
        std::ostringstream _stringStream;

        /** The wrapped ostream. */
        std::ostream& _stream;
    };

    /** The logging class */
    class Log : public std::ostream 
    {
    public: 
        Log() : std::ios(0), std::ostream( &_logBuffer ), _logBuffer(std::cout)
            {}
        virtual ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }

        /** The current log level. */
        static int level;

        /** The per-thread logger. */
        static Log& instance();

    private:
        LogBuffer _logBuffer; 

        Log( const Log& );
        Log& operator = ( const Log& );
    };

    /** The ostream indent manipulator. */
    std::ostream& indent( std::ostream& os );
    /** The ostream exdent manipulator. */
    std::ostream& exdent( std::ostream& os );

    inline void dumpStack( std::ostream& os )
    {
#   ifdef backtrace
        void* trace[256];
        const int n = backtrace(trace, 256);
        if (!n)
            return;

        const char** strings = backtrace_symbols (trace, n);
 
        for (int i = 0; i < n; ++i)
            os << i << ": " << strings[i] << std::endl;
        if (strings)
            free (strings);
#   else // backtrace
#   endif // backtrace
    }
}

#ifdef WIN32
#define SUBDIR " "
#endif

#ifdef NDEBUG
#  define LOG_EXTRA << getpid()  << "." << pthread_self()  \
        << " " << SUBDIR <<"/" << __FILE__ << ":" << (int)__LINE__ << " " 
#else

extern eqBase::Clock eqLogClock;

#  define LOG_EXTRA << getpid()  << "." << pthread_self()               \
        << " " << SUBDIR <<"/" << __FILE__ << ":" << (int)__LINE__ << " t:" \
        << eqLogClock.getMSf() << " "
#endif

#define EQERROR (eqBase::Log::level >= eqBase::LOG_ERROR) && \
    eqBase::Log::instance() << "E " LOG_EXTRA
#define EQWARN  (eqBase::Log::level >= eqBase::LOG_WARN)  && \
    eqBase::Log::instance() << "W "  LOG_EXTRA
#define EQINFO  (eqBase::Log::level >= eqBase::LOG_INFO)  && \
    eqBase::Log::instance() << "I "  LOG_EXTRA
#define EQVERB  (eqBase::Log::level >= eqBase::LOG_VERBATIM)  && \
    eqBase::Log::instance() << "V "  LOG_EXTRA

#define LOG_MATRIX4x4( m ) endl \
 << "  " << m[0] << " " << m[4] << " " << m[8]  << " " << m[12] << " " << endl \
 << "  " << m[1] << " " << m[5] << " " << m[9]  << " " << m[13] << " " << endl \
 << "  " << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << " " << endl \
 << "  " << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << " " << endl

#define LOG_VECTOR6( v ) \
    "[" << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << " " \
    << v[4] << " " << v[5] << "]"

#endif //EQBASE_LOG_H

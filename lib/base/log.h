
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <time.h>
#include <unistd.h>

#include "clock.h"

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

    /** The logging topics. */
    enum LogTopic
    {
        LOG_CUSTOM = 0x10
    };

    /** The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
	public:
		LogBuffer( std::ostream& stream )
                : _line(0), _indent(0), _noFlush(0), _noHeader(0), 
                  _newLine(true), _stream(stream)
            {}
        
        void indent() { ++_indent; }
        void exdent() { --_indent; }

        void disableFlush() { ++_noFlush; } // use counted variable to allow
        void enableFlush()                  //   nested enable/disable calls
            { 
                assert( _noFlush && "Too many enableFlush on log stream" );
                --_noFlush;
                if( _noFlush == 0 )
                    pubsync();
            }

        void disableHeader() { ++_noHeader; } // use counted variable to allow
        void enableHeader()  { --_noHeader; } //   nested enable/disable calls

        void setLogInfo( const char* subdir, const char* file, const int line )
            { _file = std::string( subdir ) + '/' + file; _line = line; }

	protected:
        virtual int_type overflow (int_type c) 
            {
                if( c == EOF )
                    return EOF;

                if( _newLine )
                {
                    if( !_noHeader )
                    {
                        _stringStream << getpid()  << "." << pthread_self()<<" "
                                      << _file << ":" << _line << " ";
#                   ifndef NDEBUG
                        _stringStream << _clock.getMSf() << " ";
#                   endif
                    }

                    for( int i=0; i<_indent; ++i )
                        _stringStream << "    ";
                    _newLine = false;
                }

                _stringStream << (char)c;
                return c;
            }
        
        virtual int sync() 
            {
                if( !_noFlush )
                {
                    const std::string& string = _stringStream.str();
                    _stream.write( string.c_str(), string.length( ));
                    _stringStream.str("");
                }
                _newLine = true;
                return 0;
            }

    private:
        LogBuffer( const LogBuffer& );
        LogBuffer& operator = ( const LogBuffer& );

        /** The current file logging. */
        std::string _file;

        /** The current line logging. */
        int _line;

        /** Clock for time stamps */
        static Clock _clock;

        /** The current indentation level. */
        int _indent;

        /** Flush reference counter. */
        int _noFlush;

        /** The header disable counter. */
        int _noHeader;

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
        Log() : std::ostream( &_logBuffer ), _logBuffer(std::cout)
            {}
        virtual ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }
        void disableFlush() { _logBuffer.disableFlush(); }
        void enableFlush()  { _logBuffer.enableFlush();  }
        void forceFlush()  { _logBuffer.pubsync();  }
        void disableHeader() { _logBuffer.disableHeader(); }
        void enableHeader()  { _logBuffer.enableHeader();  }

        /** The current log level. */
        static int level;

        /** The current log topics. */
        static unsigned topics;

        /** The per-thread logger. */
        static Log& instance( const char* subdir, const char* file,
                              const int line );

    private:
        LogBuffer _logBuffer; 

        Log( const Log& );
        Log& operator = ( const Log& );

        void setLogInfo( const char* subdir, const char* file, const int line )
            { _logBuffer.setLogInfo( subdir, file, line ); }
    };

    /** The ostream indent manipulator. */
    std::ostream& indent( std::ostream& os );
    /** The ostream exdent manipulator. */
    std::ostream& exdent( std::ostream& os );
    /** The ostream flush disable manipulator. */
    std::ostream& disableFlush( std::ostream& os );
    /** The ostream flush enable manipulator. */
    std::ostream& enableFlush( std::ostream& os );
    /** The ostream forced flush manipulator. */
    std::ostream& forceFlush( std::ostream& os );
    /** The ostream header disable manipulator. */
    std::ostream& disableHeader( std::ostream& os );
    /** The ostream header enable manipulator. */
    std::ostream& enableHeader( std::ostream& os );

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

#ifndef SUBDIR
#  define SUBDIR ""
#endif

#define EQERROR (eqBase::Log::level >= eqBase::LOG_ERROR) &&    \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQWARN  (eqBase::Log::level >= eqBase::LOG_WARN)  &&    \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQINFO  (eqBase::Log::level >= eqBase::LOG_INFO)  &&    \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQVERB  (eqBase::Log::level >= eqBase::LOG_VERBATIM)  &&    \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )
#define EQLOG(topic)  (eqBase::Log::topics && (topic))  &&  \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )

#define LOG_MATRIX4x4( m ) endl \
 << "  " << m[0] << " " << m[4] << " " << m[8]  << " " << m[12] << " " << endl \
 << "  " << m[1] << " " << m[5] << " " << m[9]  << " " << m[13] << " " << endl \
 << "  " << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << " " << endl \
 << "  " << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << " " << endl

#define LOG_VECTOR6( v ) \
    "[" << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << " " \
    << v[4] << " " << v[5] << "]"

#endif //EQBASE_LOG_H

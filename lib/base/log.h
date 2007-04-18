
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <eq/base/base.h>
#include <eq/base/clock.h>

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <time.h>

#ifdef WIN32
#  include <process.h>
#  define getpid _getpid
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
        LOG_ERROR = 1,
        LOG_WARN,
        LOG_INFO,
        LOG_VERBATIM
    };

    /** The logging topics. */
    enum LogTopic
    {
        LOG_CUSTOM = 0x10,       // 16
        LOG_ANY    = 0xfffu      // Does not include user-level events.
    };

    /** The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
	public:
		LogBuffer( std::ostream& stream )
                : _line(0), _indent(0), _blocked(0), _noHeader(0), 
                  _newLine(true), _stream(stream)
            {}
        
        void indent() { ++_indent; }
        void exdent() { --_indent; }

        void disableFlush() { ++_blocked; } // use counted variable to allow
        void enableFlush()                  //   nested enable/disable calls
            { 
                assert( _blocked && "Too many enableFlush on log stream" );
                --_blocked;
                if( !_blocked ) 
                    pubsync();
            }

        void disableHeader() { ++_noHeader; } // use counted variable to allow
        void enableHeader()  { --_noHeader; } //   nested enable/disable calls

#ifdef WIN32
        void setLogInfo( const char* subdir, const char* file, const int line )
            { _file = file; _line = line; } // SUBDIR not needed on WIN32
#else
        void setLogInfo( const char* subdir, const char* file, const int line )
            { _file = std::string( subdir ) + '/' + file; _line = line; }
#endif

	protected:
        virtual int_type overflow (int_type c) 
            {
                if( c == EOF )
                    return EOF;

                if( _newLine )
                {
                    if( !_noHeader )
                    {
#                   ifdef WIN32
                        _stringStream << getpid()  << "." << pthread_self().p <<" "
                                      << _file << ":" << _line << " ";
#                   else
                        _stringStream << getpid()  << "." << pthread_self()<<" "
                                      << _file << ":" << _line << " ";
#                   endif
#                   ifndef NDEBUG
                        const int prec  = _stringStream.precision();

                        _stringStream.precision( 4 );
                        _stringStream << std::setw(5) << _clock.getMSf() << " ";
                        _stringStream.precision( prec );
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
                if( !_blocked )
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
        static EQ_EXPORT Clock _clock;

        /** The current indentation level. */
        int _indent;

        /** Flush reference counter. */
        int _blocked;

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
        EQ_EXPORT Log() : std::ostream( &_logBuffer ), _logBuffer(std::cout)
            {}
        virtual EQ_EXPORT ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }
        void disableFlush() { _logBuffer.disableFlush(); }
        void enableFlush()  { _logBuffer.enableFlush();  }
        void forceFlush()  { _logBuffer.pubsync();  }
        void disableHeader() { _logBuffer.disableHeader(); }
        void enableHeader()  { _logBuffer.enableHeader();  }

        /** The current log level. */
        static EQ_EXPORT int level;

        /** The current log topics. */
        static EQ_EXPORT unsigned topics;

        /** The per-thread logger. */
        static EQ_EXPORT Log& instance( const char* subdir, const char* file,
                                        const int line );

        /** The string representation of the current log level. */
        static std::string& getLogLevelString();

    private:
        LogBuffer _logBuffer; 

        Log( const Log& );
        Log& operator = ( const Log& );

        void setLogInfo( const char* subdir, const char* file, const int line )
            { _logBuffer.setLogInfo( subdir, file, line ); }
    };

    /** The ostream indent manipulator. */
    EQ_EXPORT std::ostream& indent( std::ostream& os );
    /** The ostream exdent manipulator. */
    EQ_EXPORT std::ostream& exdent( std::ostream& os );
    /** The ostream flush disable manipulator. */
    EQ_EXPORT std::ostream& disableFlush( std::ostream& os );
    /** The ostream flush enable manipulator. */
    EQ_EXPORT std::ostream& enableFlush( std::ostream& os );
    /** The ostream forced flush manipulator. */
    EQ_EXPORT std::ostream& forceFlush( std::ostream& os );
    /** The ostream header disable manipulator. */
    EQ_EXPORT std::ostream& disableHeader( std::ostream& os );
    /** The ostream header enable manipulator. */
    EQ_EXPORT std::ostream& enableHeader( std::ostream& os );

#ifdef WIN32
    inline std::string getErrorString( const DWORD error )
    {
        char text[512] = "";
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
        const size_t length = strlen( text );
        if( length>2 && text[length-2] == '\r' )
            text[length-2] = '\0';
        return std::string( text );
    }
#endif
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
#define EQLOG(topic)  (eqBase::Log::topics & (topic))  &&  \
    eqBase::Log::instance( SUBDIR, __FILE__, __LINE__ )

#endif //EQBASE_LOG_H

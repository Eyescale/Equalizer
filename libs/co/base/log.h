
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file base/log.h
 * 
 * This file contains the logging classes for Equalizer. The macros EQERROR,
 * EQWARN, EQINFO and EQVERB output messages at their respective logging level,
 * if the level is active. They use a per-thread co::base::Log instance, which is a
 * std::ostream. EQVERB is always inactive in release builds.
 */

#ifndef COBASE_LOG_H
#define COBASE_LOG_H

#include <co/base/api.h>

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <time.h>

namespace co
{
namespace base
{
    class Clock;
    class Lock;

    /** The logging levels. @version 1.0 */
    enum LogLevel
    {
        LOG_ERROR = 1, //!< Output critical errors
        LOG_WARN,      //!< Output potentially critical warnings
        LOG_INFO,      //!< Output informational messages
        LOG_VERB,      //!< Be noisy
        LOG_ALL
    };

    /** 
     * The logging topics.
     * 
     * @sa net/log.h, client/log.h
     * @version 1.0
     */
    enum LogTopic
    {
        LOG_PLUGIN = 0x1,        //!< Plugin usage (1)
        LOG_CUSTOM = 0x10,       //!< Log topics for other namespaces start here
        LOG_ANY    = 0xffffu     //!< Log all Equalizer topics
    };

    /** @internal The string buffer used for logging. */
    class LogBuffer : public std::streambuf
    {
    public:
        LogBuffer( std::ostream& stream )
                : _line(0), _indent(0), _blocked(0), _noHeader(0),
                  _newLine(true), _stream(stream)
            { _thread[0] = 0; }
        virtual ~LogBuffer() {}

        void indent() { ++_indent; }
        void exdent() { --_indent; }

        void disableFlush() { ++_blocked; } // use counted variable to allow
        void enableFlush()                  //   nested enable/disable calls
            { 
                assert( _blocked );// Too many enableFlush on log stream
                --_blocked;
            }

        void disableHeader() { ++_noHeader; } // use counted variable to allow
        void enableHeader()  { --_noHeader; } //   nested enable/disable calls

        COBASE_API void setThreadName( const std::string& name );
        const char* getThreadName() const { return _thread; }

        void setLogInfo( const char* file, const int line )
            { _file = file; _line = line; }

    protected:
        virtual int_type overflow( LogBuffer::int_type c );
        
        virtual int sync();

    private:
        LogBuffer( const LogBuffer& );
        LogBuffer& operator = ( const LogBuffer& );

        /** Short thread name. */
        char _thread[12];

        /** The current file logging. */
        std::string _file;

        /** The current line logging. */
        int _line;

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

        /** The write lock. */
        static Lock _lock;
    };

    /** The logging class. @internal */
    class Log : public std::ostream
    {
    public:

        Log() : std::ostream( &_logBuffer ), _logBuffer( getOutput( )){}
        virtual ~Log() { _logBuffer.pubsync(); }

        void indent() { _logBuffer.indent(); }
        void exdent() { _logBuffer.exdent(); }
        void disableFlush() { _logBuffer.disableFlush(); }
        void enableFlush()  { _logBuffer.enableFlush();  }
        void forceFlush()  { _logBuffer.pubsync();  }
        void disableHeader() { _logBuffer.disableHeader(); }
        void enableHeader()  { _logBuffer.enableHeader();  }

        /** The current log level. */
        static COBASE_API int level;

        /** The current log topics. */
        static COBASE_API unsigned topics;

        /** The per-thread logger. */
        static COBASE_API Log& instance();

        /** The per-thread logger. */
        static COBASE_API Log& instance( const char* file, const int line );

        /** Exit the log instance for the current thread. */
        static COBASE_API void exit();

        /** The string representation of the current log level. */
        static std::string& getLogLevelString();

        /** Change the output stream. */
        static COBASE_API void setOutput( std::ostream& stream );

        /** Get the current output stream. @internal */
        static COBASE_API std::ostream& getOutput ();

        /**
         * Set the reference clock.
         *
         * The clock will be used instantly by all log outputs. Use 0 to reset
         * the clock to the default clock.
         *
         * @param clock the reference clock.
         */
        static COBASE_API void setClock( Clock* clock );

        /** @internal */
        void setThreadName( const std::string& name )
            { _logBuffer.setThreadName( name ); }

        /** @internal */
        const char* getThreadName() const { return _logBuffer.getThreadName(); }

    private:
        LogBuffer _logBuffer; 

        Log( const Log& );
        Log& operator = ( const Log& );

        void setLogInfo( const char* file, const int line )
            { _logBuffer.setLogInfo( file, line ); }
    };

    /** 
     * Increases the indentation level of the Log stream, causing subsequent
     * lines to be intended by four characters.
     * @version 1.0
     */
    COBASE_API std::ostream& indent( std::ostream& os );
    /** Decrease the indent of the Log stream. @version 1.0 */
    COBASE_API std::ostream& exdent( std::ostream& os );

    /** Disable flushing of the Log stream. @version 1.0 */
    COBASE_API std::ostream& disableFlush( std::ostream& os );
    /** Re-enable flushing of the Log stream. @version 1.0 */
    COBASE_API std::ostream& enableFlush( std::ostream& os );
    /** Flush the Log stream regardless of the auto-flush state. @version 1.0 */
    COBASE_API std::ostream& forceFlush( std::ostream& os );

    /** Disable printing of the Log header for subsequent lines. @version 1.0 */
    COBASE_API std::ostream& disableHeader( std::ostream& os );
    /** Re-enable printing of the Log header. @version 1.0 */
    COBASE_API std::ostream& enableHeader( std::ostream& os );
}
}

/** Output an error message to the per-thread Log stream. @version 1.0 */
#define EQERROR (co::base::Log::level >= co::base::LOG_ERROR) &&    \
    co::base::Log::instance( __FILE__, __LINE__ )
/** Output a warning message to the per-thread Log stream. @version 1.0 */
#define EQWARN  (co::base::Log::level >= co::base::LOG_WARN)  &&    \
    co::base::Log::instance( __FILE__, __LINE__ )
/** Output an informational message to the per-thread Log. @version 1.0 */
#define EQINFO  (co::base::Log::level >= co::base::LOG_INFO)  &&    \
    co::base::Log::instance( __FILE__, __LINE__ )

#ifdef NDEBUG
#  define EQVERB if( false )                                    \
        co::base::Log::instance( __FILE__, __LINE__ )
#else
/** Output a verbatim message to the per-thread Log stream. @version 1.0 */
#  define EQVERB (co::base::Log::level >= co::base::LOG_VERB)  &&    \
    co::base::Log::instance( __FILE__, __LINE__ )
#endif

/**
 * Output a message pertaining to a topic to the per-thread Log stream.
 * @version 1.0
 */
#define EQLOG(topic)  (co::base::Log::topics & (topic))  &&  \
    co::base::Log::instance( __FILE__, __LINE__ )

#endif //COBASE_LOG_H

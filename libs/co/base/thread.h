
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_THREAD_H
#define COBASE_THREAD_H

#include <co/base/api.h>      // COBASE_API definition
#include <co/base/debug.h>    // debug macros in thread-safety checks
#include <co/base/lock.h>     // member
#include <co/base/monitor.h>  // member
#include <co/base/threadID.h> // member

#include <ostream>

namespace co
{
namespace base
{
    /** An utility class to execute code in a separate execution thread. */
    class Thread 
    {
    public:
        /** Enumeration values for thread affinity. */
        enum Affinity
        {
            /* Not yet implemented:  OFF = 0, */
            CORE = 1, //!< Bind to a specific CPU core
            CPU = -65536, //!< Bind to all cores of a specific CPU
            CPU_MAX = -1024 //!< Highes bindable CPU
        };

        /** Construct a new thread. @version 1.0 */
        COBASE_API Thread();

        /** Copy constructor. @version 1.1.2 */
        COBASE_API Thread( const Thread& from );

        /** Destruct the thread. @version 1.0 */
        COBASE_API virtual ~Thread();

        /** 
         * Start the thread.
         *
         * @return <code>true</code> if the thread was launched and initialized
         *         successfully, <code>false</code> otherwise.
         * @sa init(), run()
         * @version 1.0
         */
        COBASE_API bool start();

        /** 
         * The init function for the child thread.
         *
         * The parent thread will not be unlocked before this function has been
         * executed. If the thread initialization fails, that is, this method
         * does return false, the thread will be stopped and the start() method
         * will return false.
         * 
         * @return the success value of the thread initialization.
         * @version 1.0
         */
        virtual bool init(){ return true; }

        /** 
         * The entry function for the child thread.
         *
         * This method should contain the main execution routine for the thread
         * and is called after a successful init().
         * 
         * @version 1.0
         */
        virtual void run() = 0;

        /** 
         * Exit the child thread immediately.
         * 
         * This function does not return. It is only to be called from the child
         * thread.
         *
         * @version 1.0
         */
        COBASE_API virtual void exit();

        /** 
         * Cancel (stop) the child thread.
         *
         * This function is not to be called from the child thread.
         * @version 1.0
         */
        COBASE_API void cancel();

        /** 
         * Wait for the exit of the child thread.
         *
         * @return true if the thread was joined, false otherwise.
         * @version 1.0
         */
        COBASE_API bool join();

        /** 
         * Return if the thread is stopped.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return true if the thread is stopped, false if not.
         * @version 1.0
         */
        bool isStopped() const { return ( _state == STATE_STOPPED ); }

        /** 
         * Return if the thread is running.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return true if the thread is running, false if not.
         * @version 1.0
         */
        bool isRunning() const { return ( _state == STATE_RUNNING ); }

        /** 
         * @return true if the calling thread is the same thread as this
         *         thread, false otherwise.
         * @version 1.0
         */
        COBASE_API bool isCurrent() const;

        /** @return a unique identifier for the calling thread. @version 1.0 */
        COBASE_API static ThreadID getSelfThreadID();

        /** @internal */
        COBASE_API static void yield();

        /** @internal */
        static void pinCurrentThread();

        /** @internal */
        COBASE_API static void setName( const std::string& name );

        /** @internal
         * Set the affinity of the calling thread.
         *
         * If given a value greater or equal than CORE, this method binds the
         * calling thread to core affinity - CORE. If set to a value greater
         * than CPU and smaller than 0, this method binds the calling thread to
         * all cores of the given processor (affinity - CPU).
         *
         * @param affinity the affinity value (see above).
         */
        COBASE_API static void setAffinity( const int32_t affinity );

    private:
        ThreadID _id;

        /** The current state of this thread. */
        enum State
        {
            STATE_STOPPED,
            STATE_STARTING, // start() in progress
            STATE_RUNNING,
            STATE_STOPPING  // child no longer active, join() not yet called
        };

        Monitor< State > _state;

        static void* runChild( void* arg );
        void        _runChild();

        void _installCleanupHandler();

        static void _notifyStarted();
        static void _notifyStopping();
        friend void _notifyStopping( void* ); //!< @internal

        static std::vector< int > _getCores( const int32_t affinity );

        friend std::ostream& operator << ( std::ostream& os, const Thread* );
    };

    /** Print the thread to the given output stream. */
    std::ostream& operator << ( std::ostream& os, const Thread* thread );

// thread-safety checks
// These checks are for development purposes, to check that certain objects are
// properly used within the framework. Leaving them enabled during application
// development may cause false positives, e.g., when threadsafety is ensured
// outside of the objects by the application.

#ifndef NDEBUG
#  define EQ_CHECK_THREADSAFETY
#endif

/** Declare a thread id variable to be used for thread-safety checks. */
#define EQ_TS_VAR( NAME )                                   \
public:                                                     \
    struct NAME ## Struct                                   \
    {                                                       \
        NAME ## Struct ()                                   \
            : extMutex( false ), inRegion( false )          \
            {}                                              \
        mutable co::base::ThreadID id;                      \
        mutable std::string name;                           \
        bool extMutex;                                      \
        mutable bool inRegion;                              \
    } NAME;                                                 \
private:

#ifdef EQ_CHECK_THREADSAFETY
#  define EQ_TS_RESET( NAME ) NAME.id = co::base::ThreadID::ZERO; 

#  define EQ_TS_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id == co::base::ThreadID::ZERO )                       \
        {                                                               \
            NAME.id = co::base::Thread::getSelfThreadID();              \
            NAME.name = co::base::Log::instance().getThreadName();      \
            EQVERB << "Functions for " << #NAME                         \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !NAME.extMutex && NAME.id != co::base::Thread::getSelfThreadID( )) \
        {                                                               \
            EQERROR << "Threadsafety check for " << #NAME               \
                    << " failed on object of type "                     \
                    << co::base::className( this ) << ", thread "       \
                    << co::base::Thread::getSelfThreadID() << " ("      \
                    << co::base::Log::instance().getThreadName() << ") != " \
                    << NAME.id << " (" << NAME.name << ")" << std::endl; \
            EQABORT( "Non-threadsave code called from two threads" );   \
        }                                                               \
    }

#  define EQ_TS_NOT_THREAD( NAME )                                      \
    {                                                                   \
        if( !NAME.extMutex && NAME.id != co::base::ThreadID::ZERO )     \
        {                                                               \
            if( NAME.id == co::base::Thread::getSelfThreadID( ))        \
            {                                                           \
                EQERROR << "Threadsafety check for not " << #NAME       \
                        << " failed on object of type "                 \
                        << co::base::className( this ) << std::endl;    \
                EQABORT( "Code called from wrong thread" );             \
            }                                                           \
        }                                                               \
    }

    /** @cond IGNORE */
    template< typename T > class ScopedThreadCheck : public NonCopyable
    {
    public:
        explicit ScopedThreadCheck( const T& data )
                : _data( data )
            {
                EQASSERTINFO( !data.inRegion,
                              "Another thread already in critical region" );
                data.inRegion = true;
            }
        ~ScopedThreadCheck() 
            {
                EQASSERTINFO( _data.inRegion,
                              "Another thread was in critical region" );
                _data.inRegion = false;
            }
    private:
        const T& _data;
    };
    /** @endcond */

# define EQ_TS_SCOPED( NAME ) \
    co::base::ScopedThreadCheck< NAME ## Struct > scoped ## NAME ## Check(NAME);

#else
#  define EQ_TS_RESET( NAME ) {}
#  define EQ_TS_THREAD( NAME ) {}
#  define EQ_TS_NOT_THREAD( NAME ) {}
#  define EQ_TS_SCOPED( NAME ) {}
#endif

}
}
#endif //COBASE_THREAD_H

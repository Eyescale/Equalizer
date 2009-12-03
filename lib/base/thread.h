
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_THREAD_H
#define EQBASE_THREAD_H

#ifdef EQUALIZER_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif

#include <eq/base/base.h>     // EQ_EXPORT definition
#include <eq/base/lock.h>     // member
#include <eq/base/monitor.h> // member

#include <vector>
#include <typeinfo>
#include <sstream>

namespace eq
{
namespace base
{
    class ExecutionListener;
    class ThreadPrivate;

    /** An utility class to execute code in a separate execution thread. */
    class Thread 
    {
    public:
        /** Construct a new thread. @version 1.0 */
        EQ_EXPORT Thread();

        /** Destruct the thread. @version 1.0 */
        EQ_EXPORT virtual ~Thread();

        /** 
         * Start the thread.
         *
         * All thread state listeners will be notified from the new thread,
         * after the thread was initialized successfully.
         * 
         * @return <code>true</code> if the thread was launched and initialized
         *         successfully, <code>false</code> otherwise.
         * @sa init(), run(), addListener()
         * @version 1.0
         */
        EQ_EXPORT bool start();

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
         * @return the return value of the child thread.
         * @version 1.0
         */
        virtual void* run() = 0;

        /** 
         * Exit the child thread immediately.
         * 
         * This function does not return. It is only to be called from the child
         * thread. The thread listeners will be notified.
         *
         * @param retVal the return value of the thread.
         * @version 1.0
         */
        EQ_EXPORT virtual void exit( void* retVal = 0 );

        /** 
         * Cancel (stop) the child thread.
         *
         * This function is not to be called from the child thread.
         * @version 1.0
         */
        EQ_EXPORT void cancel();

        /** 
         * Wait for the exit of the child thread.
         *
         * @param retVal output value for the return value of the child, can be
         *               <code>0</code>.
         * @return true if the thread was joined, false otherwise.
         * @version 1.0
         */
        EQ_EXPORT bool join( void** retVal=0 );

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
         * Returns if this thread object is the current (calling) thread.
         * 
         * @return true if the current thread has is the same thread as this
         *         thread, false if not.
         * @version 1.0
         */
        EQ_EXPORT bool isCurrent() const;

        /** 
         * Add a new thread state listener.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        EQ_EXPORT static void addListener( ExecutionListener* listener );

        /** 
         * Remove a thread state listener.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        EQ_EXPORT static bool removeListener( ExecutionListener* listener );

        /** Remove all registered listeners, used at exit. @version 1.0 */
        EQ_EXPORT static void removeAllListeners();

        /** @return a unique identifier for the calling thread. @version 1.0 */
        EQ_EXPORT static size_t getSelfThreadID();

        /** @internal */
        static void pinCurrentThread();

    private:
        ThreadPrivate* _data;

        /** The current state of this thread. */
        enum State
        {
            STATE_STOPPED,
            STATE_STARTING, // start() in progress
            STATE_RUNNING,
            STATE_STOPPING  // child no longer active, join() not yet called
        };

        Monitor< State > _state;

#ifdef EQ_WIN32_SDP_JOIN_WAR
        void* _retVal;
#endif

        static void* runChild( void* arg );
        void        _runChild();

        void _installCleanupHandler();

        static void _notifyStarted();
        static void _notifyStopping();
        friend void _notifyStopping( void* ); //!< @internal

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
#define CHECK_THREAD_DECLARE( NAME )                        \
    struct NAME ## Struct                                   \
    {                                                       \
        NAME ## Struct ()                                   \
            : id( 0 ), extMutex( false ), inRegion( false ) \
            {}                                              \
        mutable size_t id;                                  \
        bool extMutex;                                      \
        bool inRegion;                                      \
    } NAME;                                                 \

#ifdef EQ_CHECK_THREADSAFETY
#  define CHECK_THREAD_RESET( NAME ) NAME.id = 0;

#  define CHECK_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id == 0 )                                              \
        {                                                               \
            NAME.id = eq::base::Thread::getSelfThreadID();              \
            EQVERB << "Functions for " << #NAME                         \
                   << " locked to this thread" << std::endl;            \
        }                                                               \
        if( !NAME.extMutex && NAME.id != eq::base::Thread::getSelfThreadID( )) \
        {                                                               \
            EQERROR << "Threadsafety check for " << #NAME               \
                    << " failed on object of type "                     \
                    << typeid(*this).name() << std::endl;               \
            EQABORT( "Non-threadsave code called from two threads" );   \
        }                                                               \
    }

#  define CHECK_NOT_THREAD( NAME )                                      \
    {                                                                   \
        if( !NAME.extMutex && NAME.id != 0 )                           \
        {                                                               \
            if( NAME.id ==  eq::base::Thread::getSelfThreadID( ))         \
            {                                                           \
                EQERROR << "Threadsafety check for not " << #NAME       \
                        << " failed on object of type "                 \
                        << typeid(*this).name() << std::endl;           \
                EQABORT( "Code called from wrong thread" );             \
            }                                                           \
        }                                                               \
    }

    /** @internal */
    template< typename T > class ScopedThreadCheck : public NonCopyable
    {
    public:
        explicit ScopedThreadCheck( T& data )
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
        T& _data;
    };

# define CHECK_THREAD_SCOPED( NAME ) \
    eq::base::ScopedThreadCheck< NAME ## Struct > scoped ## NAME ## Check(NAME);

#else
#  define CHECK_THREAD_RESET( NAME ) {}
#  define CHECK_THREAD( NAME ) {}
#  define CHECK_NOT_THREAD( NAME ) {}
#  define CHECK_THREAD_SCOPED( NAME ) {}
#endif

}
}
#endif //EQBASE_THREAD_H

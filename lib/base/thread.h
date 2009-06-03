
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

#include <eq/base/base.h>     // EQ_EXPORT definition
#include <eq/base/lock.h>     // member

#ifdef EQ_WIN32_SDP_JOIN_WAR
#  include <eq/base/monitor.h> // member
#endif

#include <vector>
#include <typeinfo>
#include <sstream>

namespace eq
{
namespace base
{
    class ExecutionListener;
    class ThreadPrivate;

    /**
     * An abstraction for an execution thread.
     */
    class Thread 
    {
    public:
        /** 
         * Constructs a new thread.
         */
        EQ_EXPORT Thread();

        /** Destructs the thread. */
        EQ_EXPORT virtual ~Thread();

        /** 
         * Starts the thread.
         *
         * All thread state listeners will be notified from the running thread,
         * after the thread was initialized successfully.
         * 
         * @return <code>true</code> if the thread was launched,
         *         <code>false</code> otherwise.
         * @sa init(), run(), addListener()
         */
        EQ_EXPORT bool start();

        /** 
         * The init function for the child thread.
         *
         * The parent thread will not be unlocked before this function has been
         * executed. If the thread initialization fails, that is, this method
         * did return false, the thread will be stopped and the start() method
         * will return false.
         * 
         * @return the success value of the thread initialization.
         */
        virtual bool init(){ return true; }

        /** 
         * The entry function for the child thread.
         *
         * This method should contain the main execution routine for the thread
         * and is called after a successful init().
         * 
         * @return the return value of the child thread.
         */
        virtual void* run() = 0;

        /** 
         * Exit the child thread immediately.
         * 
         * This function does not return. It is only to be called from the child
         * thread. The thread listeners will be notified.
         *
         * @param retVal the return value of the thread.
         */
        EQ_EXPORT virtual void exit( void* retVal = 0 );

        /** 
         * Cancel (stop) the child thread.
         *
         * This function is not to be called from the child thread.
         */
        EQ_EXPORT void cancel();

        /** 
         * Wait for the exit of the child thread.
         *
         * @param retVal output value for the return value of the child, can be
         *               <code>0</code>.
         * @return <code>true</code> if the thread was joined,
         *         <code>false</code> otherwise.
         */
        EQ_EXPORT bool join( void** retVal=0 );

        /** 
         * Return if the thread is stopped.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return <code>true</code> if the thread is stopped,
         * <code>false</code> if not.
         */
        bool isStopped() const { return ( _state == STATE_STOPPED ); }

        /** 
         * Return if the thread is running.
         * 
         * Note that the thread may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return <code>true</code> if the thread is running,
         * <code>false</code> if not.
         */
        bool isRunning() const { return ( _state == STATE_RUNNING ); }

        /** 
         * Returns if this thread object is the current (calling) thread.
         * 
         * @return <code>true</code> if the current thread has is the same
         *         thread as this thread, <code>false</code> if not.
         */
        EQ_EXPORT bool isCurrent() const;

        /** 
         * Add a new thread state listener.
         * 
         * @param listener the listener.
         */
        EQ_EXPORT static void addListener( ExecutionListener* listener );

        /** 
         * Remove a thread state listener.
         * 
         * @param listener the listener.
         */
        EQ_EXPORT static bool removeListener( ExecutionListener* listener );

        /** Remove all registered listeners, used at exit. */
        EQ_EXPORT static void removeAllListeners();

        /** @return a unique identifier for the calling thread. */
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

        State _state;
        Lock  _syncChild;

#ifdef EQ_WIN32_SDP_JOIN_WAR
        Monitor<bool> _running;
        void*         _retVal;
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

/** Declare a thread id variable to be used for thread-safety checks. */
#define CHECK_THREAD_DECLARE( NAME )                        \
    struct NAME ## Struct                                   \
    {                                                       \
        NAME ## Struct ()                                   \
            : id( 0 ), extMutex( false )                    \
            {}                                              \
        mutable size_t id;                                  \
        bool extMutex;                                      \
    } NAME;                                                 \

#ifdef EQ_CHECK_THREADSAFETY
#  define CHECK_THREAD_RESET( NAME ) NAME.id = 0;

#  define CHECK_THREAD( NAME )                                          \
    {                                                                   \
        if( NAME.id == 0 )                                              \
        {                                                               \
            NAME.id = eq::base::Thread::getSelfThreadID();                \
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
#else
#  define CHECK_THREAD_RESET( NAME )
#  define CHECK_THREAD( NAME )
#  define CHECK_NOT_THREAD( NAME )
#endif

}
}
#endif //EQBASE_THREAD_H

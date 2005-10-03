
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_THREAD_H
#define EQBASE_THREAD_H

#include "base.h"

namespace eqBase
{
    class Lock;

    /**
     * An abstraction to create a new execution thread.
     * 
     * Depending on the type, a different implementation is used to create the
     * thread. Please note that certain implementations, e.g., fork, create the
     * working unit in another address space. 
     */
    class Thread {

    public:
        /** The thread types. */
        enum Type
        {
            PTHREAD,
            FORK
        };

        /** 
         * Constructs a new thread.
         * 
         * @param type the execution model to use for the thread.
         */
        Thread( const Type type = PTHREAD );

        /** Destructs the thread. */
        virtual ~Thread();

        /** 
         * Starts the thread using the specified entry function.
         * 
         * @return <code>true</code> if the thread was launched,
         *         <code>false</code> otherwise.
         */
        bool start();

        /** 
         * The init function for the child thread.
         *
         * The parent thread will not be unlocked before this function has been
         * executed. If the thread initialisation fails, that is, this method
         * did return false, the thread will be stopped and the start() method
         * will return false.
         * 
         * @return the success value of the thread initialisation.
         */
        virtual bool init(){ return true; }

        /** 
         * The entry function for the child thread.
         * 
         * @return the return value of the child thread.
         */
        virtual ssize_t run() = 0;

        /** 
         * Exits the child thread immediately.
         * 
         * If successful, this function does not return.
         *
         * @param retVal the return value of the thread.
         */
        void exit( ssize_t retVal = NULL );

        /** 
         * Waits for the exit of the child thread.
         * 
         * The actual size of the return value is thread-type dependent and may
         * be as low as 8 bits.
         *
         * @param retVal output value for the return value of the child, can be
         *               <code>NULL</code>.
         * @return <code>true</code> if the thread was joined,
         *         <code>false</code> otherwise.
         */
        bool join( ssize_t* retVal=NULL );

        /** 
         * Returns the execution type for this thread.
         * 
         * @return the execution type for this thread.
         */
        Type getType() { return _type; }

        /** 
         * Sets a thread-specific data for the current thread.
         * 
         * @param data the thread-specific data.
         */
        static void setSpecific( void* data );

        /** 
         * Gets the thread-specific data for the current thread.
         * 
         * 
         * @return the thread-specific data.
         */
        static void* getSpecific();

    private:
        /** The current state of this thread. */
        enum State
        {
            STATE_STOPPED,
            STATE_STARTING, // start() in progress
            STATE_RUNNING,
            STATE_STOPPING  // child no longer active, join() not yet called
        };

        Type  _type;
        State _threadState;
        Lock* _lock;

        union ThreadID
        {
            pthread_t pthread;
            pid_t     fork;
        };

        ThreadID             _threadID;
        ssize_t              _retVal;
        static pthread_key_t _dataKey;
        static bool          _dataKeyCreated;

        static void* runChild( void* arg );
        void        _runChild();

        ThreadID    _getLocalThreadID();
        static bool _createDataKey();
    };
}

#endif //EQBASE_THREAD_H

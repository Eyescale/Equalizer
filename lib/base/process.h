
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_PROCESS_H
#define EQBASE_PROCESS_H

#include "base.h"

#include <vector>

namespace eqBase
{
    class Lock;
    class RunnableListener;

    /**
     * An abstraction to create a new execution process.
     */
    class Process 
    {
    public:
        /** 
         * Constructs a new process.
         */
        Process();

        /** Destructs the process. */
        virtual ~Process();

        /** 
         * Starts the process.
         *
         * All process listeners will be notified from within the process, after
         * the process was initialized successfully.
         * 
         * @return <code>true</code> if the process was launched,
         *         <code>false</code> otherwise.
         * @sa init(), run()
         */
        bool start();

        /** 
         * The init function for the child process.
         *
         * @todo: The parent process will not be unlocked before this function
         * has been executed. If the process initialisation fails, that is, this
         * method did return false, the process will be stopped and the start()
         * method will return false.
         * 
         * @return the success value of the process initialisation.
         */
        virtual bool init(){ return true; }

        /** 
         * The entry function for the child process.
         * 
         * @return the return value of the child process.
         */
        virtual int run() = 0;

        /** 
         * Exits the child process immediately.
         * 
         * This function does not return. It is only to be called from the child
         * process. The process listeners will be notified.
         *
         * @param retVal the return value of the process.
         */
        virtual void exit( int retVal = 0 );

        /** 
         * Cancels (stops) the child process.
         *
         * This function is not to be called from the child process.
         */
        void cancel();

        /** 
         * Waits for the exit of the child process.
         * 
         * The actual size of the return value may be as low as 8 bits.
         *
         * @param retVal output value for the return value of the child, can be
         *               <code>NULL</code>.
         * @return <code>true</code> if the process was joined,
         *         <code>false</code> otherwise.
         */
        bool join( int* retVal=NULL );

        /** 
         * Returns if the process is stopped.
         * 
         * Note that the process may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return <code>true</code> if the process is stopped,
         * <code>false</code> if not.
         */
        bool isStopped() const { return ( _state == STATE_STOPPED ); }

        /** 
         * Returns if the process is running.
         * 
         * Note that the process may be neither running nor stopped if it is
         * currently starting or stopping.
         *
         * @return <code>true</code> if the process is running,
         * <code>false</code> if not.
         */
        bool isRunning() const { return ( _state == STATE_RUNNING ); }

        /** 
         * Returns if this process is the current (calling) process.
         * 
         * @return <code>true</code> if the current process has is the same
         *         process as this process, <code>false</code> if not.
         */
        bool isCurrent() const;

        /** 
         * Add a new process state listener.
         * 
         * @todo listener implementation for processes.
         *
         * @param listener the listener.
         */
        static void addListener( RunnableListener* listener );

    private:
        /** The current state of this process. */
        enum State
        {
            STATE_STOPPED,
            STATE_STARTING, // start() in progress
            STATE_RUNNING,
            STATE_STOPPING  // child no longer active, join() not yet called
        };

        State _state;
        pid_t _pid;

        static void* runChild( void* arg );
        void        _runChild();

        // @todo listener API
    };
}

#endif //EQBASE_PROCESS_H

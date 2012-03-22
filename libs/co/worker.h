
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef CO_WORKER_H
#define CO_WORKER_H

#include <co/api.h>          // CO_API definition
#include <co/types.h>
#include <lunchbox/thread.h>  // base class

namespace co
{
    /** A worker thread. */
    template< class Q > class WorkerThread : public base::Thread
    {
    public:
        /** Construct a new worker thread. @version 1.1.5 */
        WorkerThread() {}

        /** Destruct the worker. @version 1.1.5 */
        virtual ~WorkerThread() {}

        /** @return the queue to the worker thread. @version 1.1.5 */
        Q* getWorkerQueue() { return &_commands; }

    protected:
        /** @sa base::Thread::init() */
        virtual bool init()
            {
                setName( base::className( this ));
                return true;
            }

        /** @sa base::Thread::run() */
        virtual void run();

        /** @return true to stop the worker thread. @version 1.1.5 */
        virtual bool stopRunning() { return false; }
        
        /** @return true to indicate pending idle tasks. @version 1.1.5 */
        virtual bool notifyIdle() { return false; }

    private:
        /** The receiver->worker thread command queue. */
        Q _commands;
    };

    typedef WorkerThread< CommandQueue > Worker; // instantiated in worker.cpp
}
#endif //CO_WORKER_H

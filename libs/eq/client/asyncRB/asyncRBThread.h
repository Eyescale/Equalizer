
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *                    2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_DETAIL_ASYNCRBTHREAD_H
#define EQ_DETAIL_ASYNCRBTHREAD_H

#include <eq/client/types.h>
#include <eq/client/commandQueue.h> // template type used by base class
#include <co/worker.h> // base class

namespace eq
{
namespace detail
{

/** Asynchronous, per-pipe readback thread. */
class AsyncRBThread : public eq::Worker
{
public:
    AsyncRBThread();
    ~AsyncRBThread();

    virtual bool start();
    virtual bool stopRunning() { return !_running; }
    virtual bool init();

    void setWindow( Window* window ){ _window = window; }
    void postStop();
    const GLEWContext* glewGetContext() const;

private:
    bool _running; // Async thread will exit if this is false

    Window*           _window;
    eq::SystemWindow* _sharedContextWindow;
};

}
}

#endif //EQ_DETAIL_ASYNCRBTHREAD_H

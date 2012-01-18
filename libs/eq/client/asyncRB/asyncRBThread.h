
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef EQ_ASYNC_RB_ASYNC_FETCHER_H
#define EQ_ASYNC_RB_ASYNC_FETCHER_H

#include <eq/eq.h>

namespace eq
{

/**
 *  Commands to/from RB thread
 */
enum AsyncRBCommands
{
    INITIALIZED, // from
    INIT_FAILED, // from
    EXIT         // to
};


class Window;

/**
 *  Asynchronous readback thread.
 */
class AsyncRBThread : public co::base::Thread
{
public:
    typedef eq::util::ObjectManager< int > ObjectManager;

    AsyncRBThread();
    ~AsyncRBThread();

    virtual void run();
    void setup( Window* wnd );

    /* Nonblocking */
    void startRB( Channel* channel );

    AsyncRBCommands getRespond() { return _outQueue.pop(); }
    void pushCommand( const AsyncRBCommands command )
                                 { _inQueue.push( command ); }

    const GLEWContext* glewGetContext() const;

private:
    Window*                             _wnd;
    co::base::MTQueue<AsyncRBCommands>  _inQueue;
    co::base::MTQueue<AsyncRBCommands>  _outQueue;
    eq::ObjectManager*                  _objectManager;
    eq::SystemWindow*                   _sharedContextWindow;
};

} 

#endif //EQ_ASYNC_RB_ASYNC_FETCHER_H

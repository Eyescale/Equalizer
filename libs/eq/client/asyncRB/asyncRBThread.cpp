
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
 *
 */

#include "asyncRBThread.h"

#ifdef AGL
#  include "aglWindowShared.h"
#endif
#ifdef GLX
#  include "glXWindowShared.h"
#endif

namespace eq
{
namespace detail
{

namespace
{
eq::SystemWindow* _initSharedContextWindow( eq::Window* window )
{
    EQASSERT( window );

    // store old drawable of window and set window's drawable to FBO,
    // create another (shared) osWindow and restore original drowable
    const int32_t drawable =
        window->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    const int32_t stencil =
        window->getIAttribute( eq::Window::IATTR_PLANES_STENCIL );
    window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, eq::OFF );

    const eq::Pipe* pipe = window->getPipe();
    EQASSERT( pipe );

    eq::SystemWindow* sharedContextWindow = 0;

    const std::string& ws = pipe->getWindowSystem().getName();

#ifdef GLX
    if( ws == "GLX" )
    {
        EQINFO << "Using GLXWindow" << std::endl;
        sharedContextWindow = new GLXWindowShared( window );
    }
#endif
#ifdef AGL
    if( ws == "AGL" )
    {
        EQINFO << "Using AGLWindow" << std::endl;
        sharedContextWindow = new AGLWindowShared( window );
    }
#endif
#ifdef WGL
    if( ws == "WGL" )
    {
        EQINFO << "Using WGLWindow" << std::endl;
        sharedContextWindow = new eq::wgl::Window( window );
    }
#endif
    if( !sharedContextWindow )
    {
        EQERROR << "Window system " << pipe->getWindowSystem()
                << " not implemented or supported" << std::endl;
        return 0;
    }

    if( !sharedContextWindow->configInit( ))
    {
        EQWARN << "OS Window initialization failed: " << std::endl;
        delete sharedContextWindow;
        return 0;
    }

    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
    window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, stencil );

    sharedContextWindow->makeCurrent();

    EQWARN << "Async fetcher initialization finished" << std::endl;
    return sharedContextWindow;
}


void _deleteSharedContextWindow( eq::Window* window,
                                 eq::SystemWindow** sharedContextWindow )
{
    EQWARN << "Deleting shared context" << std::endl;
    if( !sharedContextWindow || !*sharedContextWindow )
        return;

    const int32_t drawable =
        window->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    (*sharedContextWindow)->configExit(); // mb set window to 0 before that?

    delete *sharedContextWindow;
    *sharedContextWindow = 0;

    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
}
}

AsyncRBThread::AsyncRBThread()
    : eq::Worker()
    , _running( true )
    , _window( 0 )
    , _sharedContextWindow( 0 )
{
}


AsyncRBThread::~AsyncRBThread()
{
    if( _window && _sharedContextWindow )
        _deleteSharedContextWindow( _window, &_sharedContextWindow );
}

bool AsyncRBThread::start()
{
    if( isRunning( ))
        return true;
    return Worker::start();
}

const GLEWContext* AsyncRBThread::glewGetContext() const
{
    return _sharedContextWindow->glewGetContext();
}


void AsyncRBThread::postStop()
{
    _deleteSharedContextWindow( _window, &_sharedContextWindow );
    _running = false;
}


bool AsyncRBThread::init()
{
    co::base::Thread::setName( "async rb " + co::base::className( this ));
    EQASSERT( !_sharedContextWindow );

    if( !_window )
        return false;

    _sharedContextWindow = _initSharedContextWindow( _window );
    if( _sharedContextWindow )
        return true;

    EQERROR << "Can't init shader context window" << std::endl;
    return false;
}

}
}


/* Copyright (c) 2009-2015, Stefan.Eilemann@epfl.ch
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

#include "window.h"
#include <eq/system.h>
#include <eq/fabric/drawableConfig.h>
#include <co/objectOCommand.h>

namespace eq
{
namespace x11
{

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                Display* xDisplay )
    : SystemWindow( parent, settings )
    , _xDisplay( xDisplay )
    , _xDrawable( 0 )
{
}

Display* Window::getXDisplay()
{
    return _xDisplay;
}

bool Window::configInit()
{
    if( !_xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }

    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
    {
        const int screen = DefaultScreen( _xDisplay );
        setPixelViewport(
            PixelViewport( 0, 0, DisplayWidth( _xDisplay, screen ),
                           DisplayHeight( _xDisplay, screen )));
    }

    XID drawable = _createWindow();
    if( !drawable )
        return false;

    // Grab keyboard focus in fullscreen mode
    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
        XGrabKeyboard( _xDisplay, drawable, True, GrabModeAsync, GrabModeAsync,
                       CurrentTime );

    setXDrawable( drawable );
    return true;
}

XID Window::_createWindow()
{
    if( !_xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }

    const int screen = DefaultScreen( _xDisplay );
    XID parent = RootWindow( _xDisplay, screen );
    const PixelViewport& pvp = getPixelViewport();
    XID drawable = XCreateSimpleWindow( _xDisplay,
                                        parent,
                                        pvp.x, pvp.y, pvp.w, pvp.h, 1,
                                        BlackPixel( _xDisplay, screen ),
                                        WhitePixel( _xDisplay, screen ));
    if ( !drawable )
    {
        sendError( ERROR_GLXWINDOW_CREATECONTEXT_FAILED );
        return 0;
    }

    XSelectInput( _xDisplay, drawable, ExposureMask | KeyPressMask );
    XMapWindow( _xDisplay, drawable );

    std::stringstream windowTitle;
    windowTitle << "eqCpu Example";
    XStoreName( _xDisplay, drawable, windowTitle.str().c_str( ));

    // Register for close window request from the window manager
    Atom deleteAtom = XInternAtom( _xDisplay, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( _xDisplay, drawable, &deleteAtom, 1 );

    XMoveResizeWindow( _xDisplay, drawable, pvp.x, pvp.y, pvp.w, pvp.h );
    XFlush( _xDisplay );

    return drawable;
}

void Window::configExit()
{
    if( _xDrawable )
        XDestroyWindow( _xDisplay, _xDrawable );
    setXDrawable( 0 );
}

void Window::queryDrawableConfig( DrawableConfig& drawableConfig )
{
    drawableConfig.stencilBits = 0;
    drawableConfig.alphaBits = 0;
    drawableConfig.glVersion = 0.f;
    drawableConfig.stereo = false;
    drawableConfig.doublebuffered = false;
}

void Window::flush()
{
    XFlush( _xDisplay );
}

}
}

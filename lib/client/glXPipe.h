
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQ_OS_PIPE_GLX_H
#define EQ_OS_PIPE_GLX_H

#include <eq/client/osPipe.h> // base class

namespace eq
{
    class GLXEventHandler;

    /** Equalizer default implementation of a GLX window */
    class EQ_EXPORT GLXPipe : public OSPipe
    {
    public:
        GLXPipe( Pipe* parent );
        virtual ~GLXPipe( );

        //* @name GLX/X11 initialization
        //*{
        /** 
         * Initialize this pipe for the GLX window system.
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit( );

        /** 
         * Deinitialize this pipe for the GLX window system.
         * 
         * @return true if the deinitialization was successful, false otherwise.
         */
        virtual void configExit( );

        /** Init the GLX-specific event handler. */
        virtual void initEventHandler();
        /** Exit the GLX-specific event handler. */
        virtual void exitEventHandler();
        //*}

        /** @return the X display connection for this pipe. */
        Display* getXDisplay() const { return _xDisplay; }

        /** @return the associated event handler. */
        GLXEventHandler* getGLXEventHandler() { return _eventHandler; }

    private:
        /** 
         * Set the X display connection for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit(). Updates the pixel viewport.
         *
         * @param display the X display connection for this pipe.
         */
        void _setXDisplay( Display* display );

        /**
         * @return The string representation of this pipe's port and device
         *         setting, in the form used by XOpenDisplay().
         */
        std::string _getXDisplayString();

        //check if it should be private
        static int XErrorHandler( Display* display, XErrorEvent* event );

        /** Window-system specific display information. */
        Display* _xDisplay;

        /** The event handler for our display connection. */
        GLXEventHandler* _eventHandler;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}

#endif // EQ_OS_PIPE_GLX_H

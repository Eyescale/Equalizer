
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
                      2009, Maxim Makhinya
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

#ifndef EQ_GLX_PIPE_H
#define EQ_GLX_PIPE_H

#include <eq/client/systemPipe.h> // base class
#include <eq/client/os.h>     // X11 types

namespace eq
{
    /** Equalizer default implementation of a GLX window */
    class EQ_CLIENT_DECL GLXPipe : public SystemPipe
    {
    public:
        GLXPipe( Pipe* parent );
        virtual ~GLXPipe( );

        /** @name GLX/X11 initialization */
        //@{
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
        //@}

        /** @return the X display connection for this pipe. */
        Display* getXDisplay() const { return _xDisplay; }

    private:
        /** 
         * Set the X display connection for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit(). Updates the pixel viewport.
         *
         * @param display the X display connection for this pipe.
         * @sa XSetCurrentDisplay()
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

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}

#endif // EQ_GLX_PIPE_H


/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_PIPE_GLX_H
#define EQ_OS_PIPE_GLX_H

#include "osPipe.h"
#include "glXWindowEvent.h"

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
    };
}

#endif // EQ_OS_PIPE_GLX_H

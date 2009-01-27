
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_PIPE_WGL_H
#define EQ_OS_PIPE_WGL_H

#include <eq/client/osPipe.h>

namespace eq
{
    /** Equalizer default implementation of a WGL window */
    class EQ_EXPORT WGLPipe : public OSPipe
    {
    public:
        WGLPipe( Pipe* parent );
        virtual ~WGLPipe( );

        //* @name WGL initialization
        //*{
        /** 
         * Initialize this pipe for the WGL window system.
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit( );

        /** 
         * Deinitialize this pipe for the WGL window system.
         * 
         * @return true if the deinitialization was successful, false otherwise.
         */
        virtual void configExit( );
        //*}

        /**
         * Create a device context bound only to the display device of this
         * pipe.
         *
         * If the dc return parameter is set to 0 and the return value is true,
         * an affinity dc is not needed. The returned context has to be deleted
         * using wglDeleteDCNV when it is no longer needed.
         *
         * @param affinityDC the affinity device context output parameter.
         * @return the success status.
         */
        bool createWGLAffinityDC( HDC& affinityDC );

        /** 
         * Create a device context on the display device of this pipe.
         * 
         * The returned device context has to be deallocated using DeleteDC.
         * 
         * @return the device context, or 0 upon error.
         */
        HDC createWGLDisplayDC();

    private:

        void _configInitWGLEW();

        bool _getGPUHandle( HGPUNV& handle );
    };
}

#endif // EQ_OS_PIPE_WGL_H


/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Maxim Makhinya
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

#ifndef EQ_WGL_PIPE_H
#define EQ_WGL_PIPE_H

#include <eq/client/systemPipe.h> // base class
#include <eq/client/os.h>     // GLEW

namespace eq
{
    /** Equalizer default implementation of a WGL window */
    class EQ_EXPORT WGLPipe : public SystemPipe
    {
    public:
        WGLPipe( Pipe* parent );
        virtual ~WGLPipe( );

        /** @name WGL initialization */
        //@{
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
        //@}

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

        /** @return the generic WGL function table for the pipe. */
        WGLEWContext* wglewGetContext() { return _wglewContext; }

        /** @internal @return the NVidia driver version, or 0. */
        float getDriverVersion() const { return _driverVersion; }

    protected:
        /** Initialize this pipe for OpenGL. */
        virtual bool configInitGL();

    private:
        bool _configInitWGLEW();
        bool _getGPUHandle( HGPUNV& handle );

        /** Extended WGL function entries. */
        WGLEWContext* const _wglewContext;

        float _driverVersion;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        void _configInitDriverVersion();
    };
}

#endif // EQ_WGL_PIPE_H

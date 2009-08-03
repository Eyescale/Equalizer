
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
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

#ifndef EQ_OS_PIPE_H
#define EQ_OS_PIPE_H

#include <eq/client/windowSystem.h> // GLEW

#include <eq/base/base.h>
#include <string>

namespace eq
{
    class Pipe;

    /**
     * The interface definition for OS-specific pipe code.
     *
     * The OSPipe abstracts all pipe system specific code and facilitates
     * porting to new windowing systems. Each Pipe uses one OSPipe, which
     * is initialized in Pipe::configInitOSPipe.
     */
    class EQ_EXPORT OSPipe
    {
    public:
        OSPipe( Pipe* parent );
        virtual ~OSPipe( );

        /** @name Methods forwarded from eq::Pipe */
        //@{
        /** Initialize the GPU. */
        virtual bool configInit( ) = 0;

        /** De-initialize the GPU. */
        virtual void configExit( ) = 0;
        //@}

        /** @return the reason of the last error. */
        const std::string & getErrorMessage() const { return _error; }

        /** @return the parent Pipe. */
        Pipe* getPipe() { return _pipe; }
        
        /** @return the parent Pipe. */
        const Pipe* getPipe() const { return _pipe; }

        /** @return the generic WGL function table for the pipe. */
        WGLEWContext* wglewGetContext() { return _wglewContext; }

    protected:
        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}

        /** The parent eq::Pipe. */
        Pipe* const _pipe;

        /** The reason for the last error. */
        std::string _error;

        /** Extended OpenGL function entries - WGL. */
        WGLEWContext*   _wglewContext;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}

#endif //EQ_OS_PIPE_H


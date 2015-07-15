
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_SYSTEM_PIPE_H
#define EQ_SYSTEM_PIPE_H

#include <eq/api.h>
#include <eq/error.h> // enum
#include <eq/types.h>
#include <string>

namespace eq
{
/**
 * The interface definition for system-specific GPU handling.
 *
 * The SystemPipe abstracts all OS-system specific code for handling a GPU,
 * which facilitates porting to new windowing systems. Each Pipe uses one
 * SystemPipe, which is initialized in Pipe::configInit. The SystemPipe has to
 * set the pipe's PixelViewport if it is invalid during configInit().
 */
class SystemPipe
{
public:
    /** Create a new SstemPipe for the given eq::Pipe. @version 1.0 */
    EQ_API explicit SystemPipe( Pipe* parent );

    /** Destroy the SystemPipe. @version 1.0 */
    EQ_API virtual ~SystemPipe( );

    /** @name Methods forwarded from eq::Pipe. */
    //@{
    /** Initialize the GPU. @version 1.0 */
    EQ_API virtual bool configInit() = 0;

    /** De-initialize the GPU. @version 1.0 */
    EQ_API virtual void configExit() = 0;
    //@}

    /** @return the parent Pipe. @version 1.0 */
    Pipe* getPipe() { return _pipe; }

    /** @return the parent Pipe. @version 1.0 */
    const Pipe* getPipe() const { return _pipe; }

    /** @return the maximum available OpenGL version on this pipe.
     *  @version 1.9 */
    float getMaxOpenGLVersion() const { return _maxOpenGLVersion; }

protected:
    /** @name Error information. */
    //@{
    /**
     * Send a pipe error event to the application node.
     * @param error the error code.
     * @version 1.7.1
     */
    EQ_API EventOCommand sendError( const uint32_t error );
    //@}

    float _maxOpenGLVersion;

private:
    /** The parent eq::Pipe. */
    Pipe* const _pipe;
};
}

#endif // EQ_SYSTEM_PIPE_H

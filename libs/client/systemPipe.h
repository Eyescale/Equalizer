
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Maxim Makhinya
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

#include <eq/error.h> // enum
#include <eq/api.h>
#include <string>

namespace eq
{
    class Pipe;

    /**
     * The interface definition for system-specific GPU handling.
     *
     * The SystemPipe abstracts all OS-system specific code for handling a GPU,
     * which facilitates porting to new windowing systems. Each Pipe uses one
     * SystemPipe, which is initialized in Pipe::configInit. The SystemPipe has
     * to set the pipe's PixelViewport if it is invalid during configInit().
     */
    class SystemPipe
    {
    public:
        /** Create a new SstemPipe for the given eq::Pipe. @version 1.0 */
        EQ_API SystemPipe( Pipe* parent );

        /** Destroy the SystemPipe. @version 1.0 */
        EQ_API virtual ~SystemPipe( );

        /** @name Methods forwarded from eq::Pipe. */
        //@{
        /** Initialize the GPU. @version 1.0 */
        EQ_API virtual bool configInit( ) = 0;

        /** De-initialize the GPU. @version 1.0 */
        EQ_API virtual void configExit( ) = 0;
        //@}

        /** @return the parent Pipe. @version 1.0 */
        Pipe* getPipe() { return _pipe; }
        
        /** @return the parent Pipe. @version 1.0 */
        const Pipe* getPipe() const { return _pipe; }

        /** @return the last error. @version 1.0 */
        EQ_API co::base::Error getError() const;

    protected:
        /** @name Error information. */
        //@{
        /** 
         * Set an error code why the last operation failed.
         * @param error the error code.
         * @version 1.0
         */
        EQ_API void setError( const uint32_t error );
        //@}

    private:
        /** The parent eq::Pipe. */
        Pipe* const _pipe;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}

#endif // EQ_SYSTEM_PIPE_H


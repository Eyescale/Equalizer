
/* Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_COMPUTE_CONTEXT_H
#define EQ_COMPUTE_CONTEXT_H

#include <eq/client/api.h>
#include <eq/client/types.h>

namespace eq
{
/**
 * The interface definition for API-specific GPGPU handling.
 *
 * The ComputeContext abstracts all GPGPU API-system specific code for
 * handling a GPU for computing purposes. Each Pipe uses one ComputeContext,
 * which is initialized in Pipe::configInit.
 * @warning Experimental - may not be supported in the future.
 */
class ComputeContext
{
public:
    /**
     * Create a new ComputeContext for the given accelerator.
     */
    EQ_API ComputeContext( Pipe* parent );

    /** Destroy the ComputeContext. */
    EQ_API virtual ~ComputeContext();

    /** @name Data Access */
    //@{
    /** @return the parent pipe. */
    Pipe* getPipe() { return _pipe; }

    /** @return the parent pipe. */
    const Pipe* getPipe() const { return _pipe; }
    //@}

    /** @name Methods forwarded from Pipe */
    //@{
    /** Initialize the ComputeContext. */
    EQ_API virtual bool configInit() = 0;

    /** De-initialize the ComputeContext. */
    EQ_API virtual void configExit() = 0;
    //@}

protected:
    /** @name Error information. */
    //@{
    /**
     * Send an error event to the application node.
     *
     * @param error the error message.
     * @version 1.7.1
     */
    EventOCommand sendError( const uint32_t error );
    //@}

private:
    /** The eq::Pipe used by the context. */
    Pipe* const _pipe;
};
}

#endif // EQ_COMPUTE_CONTEXT_H

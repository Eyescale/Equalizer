
/* Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/base/base.h>

namespace eq
{
    class Pipe;

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
        EQ_EXPORT ComputeContext( Pipe* parent ); 

        /** Destroy the ComputeContext. */
        EQ_EXPORT virtual ~ComputeContext();

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
        EQ_EXPORT virtual bool configInit( ) = 0;

        /** De-initialize the ComputeContext. */
        EQ_EXPORT virtual void configExit( ) = 0;
        //@}

        /** @return the reason of the last failed operation. */
        const std::string & getErrorMessage() const { return _error; }

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
         * @version 1.0
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}

    private:
        /** The eq::Pipe used by the context. */
        Pipe* const _pipe;

        /** The reason for the last error. */
        std::string _error;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

    };		
}

#endif // EQ_COMPUTE_CONTEXT_H

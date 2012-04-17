
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_GLOBAL_H
#define CO_GLOBAL_H

#include <co/api.h>
#include <co/types.h>
#include <string>

namespace co
{
    /** Global parameter handling for the Collage library. */
    class Global
    {
    public:
        /** 
         * Sets the name of the program.
         * 
         * @param programName the program name.
         */
        CO_API static void setProgramName( const std::string& programName );

        /** @return the program name. */
        CO_API static const std::string& getProgramName();

        /** 
         * Sets the working directory of the program.
         * 
         * @param workDir the working directory.
         */
        CO_API static void setWorkDir( const std::string& workDir );

        /** @return the working directory of the program. */
        CO_API static const std::string& getWorkDir();

        /** 
         * Sets the default listening port.
         * 
         * @param port the default port.
         */
        CO_API static void setDefaultPort( const uint16_t port );

        /** @return the default listening port. */
        CO_API static uint16_t getDefaultPort();

        /** 
         * Set the minimum buffer size for Object serialization.
         *
         * The buffer size is used during serialization. When a DataOStream has
         * buffered at least size bytes, the data is send to the slave
         * nodes. The default is 60.000 bytes.
         *
         * @param size the treshold before the DataOStream sends a buffer.
         */
        CO_API static void setObjectBufferSize( const uint32_t size );

        /** @return the minimum buffer size for Object serialization. */
        CO_API static uint32_t getObjectBufferSize();

        /** 
         * Set the global variables.
         *
         * The data is expected to be a list of unsigned ints in the format
         * &#35;&#35;uint0&#35;uint1&#35;uint2&#35;...&#35;uint(n-1)&#35;&#35;
         * with n = Global::IATTR_ALL. If the data format is correct, global
         * variables will be changed and true will be returned. Otherwise there
         * will be no change and false will be returned.
         *
         * @param data the global variables in the described format
         * @return true on success, false otherwise.
         */
        CO_API static bool fromString( const std::string& data );

        /** Write global variables to data in the format for fromString(). */
        CO_API static void toString( std::string& data );

        /** @return the plugin registry. @version 1.0 */
        CO_API static PluginRegistry& getPluginRegistry();

        /** @return the error registry. @version 1.0 */
        CO_API static ErrorRegistry& getErrorRegistry();

        /** @name Attributes */
        //@{
        // Note: also update string array initialization in global.cpp
        /** Global integer attributes. */
        enum IAttribute
        {
            IATTR_INSTANCE_CACHE_SIZE,   //!< @internal max size in MB
            /** @internal send-on-register queue size */
            IATTR_NODE_SEND_QUEUE_SIZE,
            IATTR_NODE_SEND_QUEUE_AGE,   //!< @internal send-on-register max age
            IATTR_RSP_ACK_TIMEOUT,       //!< @internal time out for ack req
            IATTR_RSP_ERROR_DOWNSCALE,   //!< @internal permille per lost packet
            IATTR_RSP_ERROR_UPSCALE,     //!< @internal permille per sent packet
            IATTR_RSP_ERROR_MAXSCALE,    //!< @internal max percent change
            IATTR_RSP_MIN_SENDRATE_SHIFT, //!< @internal minBW = sendRate >> val
            IATTR_RSP_NUM_BUFFERS,       //!< @internal data buffers
            IATTR_RSP_ACK_FREQUENCY,     //!< @internal reader ack interval
            IATTR_UDP_MTU,               //!< @internal max send size on UDP
            IATTR_UDP_BUFFER_SIZE,       //!< @internal send/receiver buffer
            IATTR_QUEUE_MIN_SIZE,        //!< @internal (tile) queue min size
            IATTR_QUEUE_REFILL,          //!< @internal (tile) queue refill size
            IATTR_RDMA_RING_BUFFER_SIZE_MB, //!< @internal send/receive buffer
            IATTR_RDMA_SEND_QUEUE_DEPTH, //!< @internal max send credits
            IATTR_RDMA_RESOLVE_TIMEOUT_MS, //!< @internal address resolution
            IATTR_ROBUSTNESS,            //!< @internal use robustness
            IATTR_TIMEOUT_DEFAULT,       //!< @internal default timeout
            IATTR_ALL
        };

        /** Set an integer attribute. */
        CO_API static void setIAttribute( const IAttribute attr,
                                          const int32_t value );

        /** @return the value of an integer attribute. */
        CO_API static int32_t getIAttribute( const IAttribute attr );

        /** @return the timeout, a concrete time or LB_TIMEOUT_INDEFINITE. */
        CO_API static uint32_t getTimeout();
        //@}

        /** @return the keepalive timeout. */
        CO_API static uint32_t getKeepaliveTimeout();
        //@}
    };
}

#endif // CO_GLOBAL_H

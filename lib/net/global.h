
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_GLOBAL_H
#define EQNET_GLOBAL_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace net
{
    // global defines

    /** 
     * Global parameter handling for the Equalizer network implementation. 
     */
    class EQ_EXPORT Global
    {
    public:
        /** 
         * Sets the name of the program.
         * 
         * @param programName the program name.
         */
        static void setProgramName( const std::string& programName );

        /** @return the program name. */
        static const std::string& getProgramName() { return _programName; }

        /** 
         * Sets the working directory of the program.
         * 
         * @param workDir the working directory.
         */
        static void setWorkDir( const std::string& workDir );

        /** @return the working directory of the program. */
        static const std::string& getWorkDir() { return _workDir; }

        /** 
         * Sets the default listening port.
         * 
         * @param port the default port.
         */
        static void setDefaultPort( const uint16_t port ) 
            { _defaultPort = port; }

        /** @return the default listening port. */
        static uint16_t getDefaultPort() { return _defaultPort; }

        /** 
         * Set the minimum buffer size for Object serialization.
         *
         * The buffer size is used during serialization. When a DataOStream has
         * buffered at least size bytes, the data is send to the slave
         * nodes. The default is 60.000 bytes.
         *
         * @param size the treshold before the DataOStream sends a buffer.
         */
        static void setObjectBufferSize( const uint32_t size )
            { _objectBufferSize = size; }

        /** @return the minimum buffer size for Object serialization. */
        static uint32_t getObjectBufferSize() { return  _objectBufferSize; }

        /** @name Attributes */
        //@{
        // Note: also update string array initialization in global.cpp
        /** Global integer attributes. */
        enum IAttribute
        {
            IATTR_INSTANCE_CACHE_SIZE,   //!< @internal max size in MB 
            IATTR_RSP_ACK_TIMEOUT,       //!< @internal time out for ack req
            IATTR_RSP_MAX_TIMEOUTS,      //!< @internal timeouts before close
            IATTR_RSP_NACK_DELAY,        //!< @internal sleep before nack merge
            IATTR_RSP_ERROR_BASE_RATE,   //!< @internal normal error percentage
            IATTR_RSP_ERROR_DOWNSCALE,   //!< @internal send rate down scale (/)
            IATTR_RSP_ERROR_UPSCALE,     //!< @internal send rate up scale (*)
            IATTR_RSP_ERROR_MAX,         //!< @internal max delta for send rate
            IATTR_RSP_NUM_BUFFERS,       //!< @internal data buffers
            IATTR_UDP_MTU,               //!< @internal max send size on UDP
            IATTR_UDP_PACKET_RATE,       //!< @internal ack frequency
            IATTR_ALL
        };

        /** Set an integer attribute. */
        static void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[ attr ] = value; }

        /** @return the value of an integer attribute. */
        static int32_t getIAttribute( const IAttribute attr )
            { return _iAttributes[ attr ]; }
        //@}

    private:
        static std::string _programName;
        static std::string _workDir;
        static uint32_t    _objectBufferSize;
        static uint16_t    _defaultPort;

        /** Integer attributes. */
        static int32_t _iAttributes[IATTR_ALL];
    };
}
}

#endif // EQNET_GLOBAL_H



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

    private:
        static std::string _programName;
        static std::string _workDir;
        static uint32_t    _objectBufferSize;
        static uint16_t    _defaultPort;
    };
}
}

#endif // EQNET_GLOBAL_H


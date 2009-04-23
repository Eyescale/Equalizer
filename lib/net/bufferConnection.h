
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_BUFFER_CONNECTION_H
#define EQNET_BUFFER_CONNECTION_H

#include <eq/net/connection.h>   // base class
#include <eq/base/buffer.h>      // member

namespace eq
{
namespace net
{
    /**
     * A proxy connection buffering outgoing data into a memory region.
     */
    class EQ_EXPORT BufferConnection : public Connection
    {
    public:
        BufferConnection();
        virtual ~BufferConnection();

        void sendBuffer( ConnectionPtr connection );

        uint64_t getSize() const { return _buffer.size; }

    protected:
        virtual void readNB( void* buffer, const uint64_t bytes )
            { EQDONTCALL; }
        virtual int64_t readSync( void* buffer, const uint64_t bytes )
            { EQDONTCALL; return -1; }
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const;

    private:
        mutable base::Bufferb _buffer;
    };
}
}

#endif //EQNET_BUFFER_CONNECTION_H

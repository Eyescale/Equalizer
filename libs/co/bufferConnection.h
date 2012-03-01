
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_BUFFER_CONNECTION_H
#define CO_BUFFER_CONNECTION_H

#include <co/connection.h>  // base class
#include <co/base/types.h>

namespace co
{
namespace detail { class BufferConnection; }

    /** A proxy connection buffering outgoing data into a memory buffer. */
    class BufferConnection : public Connection
    {
    public:
        CO_API BufferConnection();
        CO_API virtual ~BufferConnection();

        CO_API void sendBuffer( ConnectionPtr connection );
        CO_API uint64_t getSize() const;

    protected:
        virtual void readNB( void*, const uint64_t ) { EQDONTCALL; }
        virtual int64_t readSync( void*, const uint64_t, const bool )
            { EQDONTCALL; return -1; }
        CO_API virtual int64_t write( const void* buffer, const uint64_t bytes);

        virtual Notifier getNotifier() const { EQDONTCALL; return 0; }

    private:
        detail::BufferConnection* const _impl;
    };
}

#endif //CO_BUFFER_CONNECTION_H

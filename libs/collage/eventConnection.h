
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_EVENT_CONNECTION_H
#define CO_EVENT_CONNECTION_H

#include <co/connection.h>   // base class

namespace co
{
    /**
     * A connection signalling an event.
     *
     * The connection is only useful to signal something to a ConnectionSet. No
     * data can be read or written from it.
     */
    class EventConnection : public Connection
    {
    public:
        CO_API EventConnection();
        CO_API virtual ~EventConnection();

        CO_API virtual bool connect();
        CO_API virtual void close();

        CO_API void set();
        CO_API void reset();

        CO_API virtual Notifier getNotifier() const;

    protected:
        virtual void readNB( void*, const uint64_t )
            { EQDONTCALL; }
        virtual int64_t readSync( void*, const uint64_t, const bool )
            { EQDONTCALL; return -1; }
        CO_API virtual int64_t write( const void*, const uint64_t )
            { EQDONTCALL; return -1; }

    private:
#ifdef WIN32
        HANDLE _event;
#else
        ConnectionPtr _connection;
	co::base::Lock _lock;
        bool _set;
#endif
    };
}

#endif //CO_EVENT_CONNECTION_H

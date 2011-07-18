
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_CONNECTIONLISTENER_H
#define CO_CONNECTIONLISTENER_H

#include <co/connection.h> // nested Connection::State enum

namespace co
{
    /** A listener interface to connection changes. */
    class ConnectionListener
    {
    public:
        virtual ~ConnectionListener() {}

        virtual void notifyStateChanged( Connection* ){}
    };
}

#endif // CO_CONNECTION_LISTENER_H

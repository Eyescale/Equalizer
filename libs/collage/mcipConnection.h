
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_MCIPCONNECTION_H
#define CO_MCIPCONNECTION_H

#include <co/connection.h>         // base class
#include <co/connectionListener.h> // base class

namespace co
{
    /** 
     * A facade for hiding different IP-based reliable multicast
     * implementations.
     */
    class MCIPConnection : public Connection, public ConnectionListener
    {
    public:
        MCIPConnection();

        virtual bool connect();
        virtual bool listen();
        virtual void close();

        virtual void acceptNB();
        virtual ConnectionPtr acceptSync();

        virtual void readNB( void* buffer, const uint64_t bytes );
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored );

        virtual Notifier getNotifier() const;

    protected:
        virtual ~MCIPConnection();

        virtual int64_t write( const void* buffer, const uint64_t bytes );

        virtual void notifyStateChanged( Connection* connection );

    private:
        ConnectionPtr _impl;
    };
}
#endif //CO_MCIPCONNECTION_H

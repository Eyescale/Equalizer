
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_PAIRCONNECTION_H
#define EQNET_PAIRCONNECTION_H

#include "connection.h"

namespace eq
{
namespace net
{
    /**
     * A meta connection consisting of two (unidirectional) connections
     * providing bidirectional communication.
     */
    class PairConnection : public Connection
    {
    public:
        EQNET_API PairConnection( ConnectionPtr readConnection,
                                  ConnectionPtr writeConnection );

        EQNET_API ConnectionPtr getSibling();

        EQNET_API virtual bool connect();
        EQNET_API virtual void close();

        virtual Notifier getNotifier() const
            { return _readConnection->getNotifier(); }

    protected:
        EQNET_API virtual ~PairConnection();

        virtual void readNB( void* buffer, const uint64_t bytes )
            { _readConnection->readNB( buffer, bytes ); }
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool block )
            { return _readConnection->readSync( buffer, bytes, block ); }
        virtual int64_t write( const void* buffer, const uint64_t bytes )
            { return _writeConnection->write( buffer, bytes ); }

    private:
        PairConnection( PairConnection* sibling );

        ConnectionPtr _readConnection;
        ConnectionPtr _writeConnection;
        base::RefPtr<PairConnection> _sibling;
    };
}
}
#endif //EQNET_PAIRCONNECTION_H

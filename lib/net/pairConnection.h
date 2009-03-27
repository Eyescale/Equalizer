
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    class EQ_EXPORT PairConnection : public Connection
    {
    public:
        PairConnection( ConnectionPtr readConnection,
                        ConnectionPtr writeConnection );

        ConnectionPtr getSibling();

        virtual bool connect();
        virtual void close();

        virtual ReadNotifier getReadNotifier() const
            { return _readConnection->getReadNotifier(); }

    protected:
        virtual ~PairConnection();

        virtual int64_t read( void* buffer, const uint64_t bytes )
            { return _readConnection.get()->read( buffer, bytes ); }
        virtual int64_t write( const void* buffer, const uint64_t bytes )
            const { return _writeConnection->write( buffer, bytes ); }

    private:
        PairConnection( PairConnection* sibling );

        ConnectionPtr _readConnection;
        ConnectionPtr _writeConnection;
        base::RefPtr<PairConnection> _sibling;
    };
}
}
#endif //EQNET_PAIRCONNECTION_H

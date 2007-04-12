
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PAIRCONNECTION_H
#define EQNET_PAIRCONNECTION_H

#include "connection.h"

namespace eqNet
{
    /**
     * A meta connection consisting of two (unidirectional) connections
     * providing bidirectional communication.
     */
    class EQ_EXPORT PairConnection : public Connection
    {
    public:
        PairConnection( eqBase::RefPtr<Connection> readConnection,
                        eqBase::RefPtr<Connection> writeConnection );

        eqBase::RefPtr<Connection> getSibling();

        virtual bool connect();
        virtual void close();

        virtual ReadNotifier getReadNotifier() 
            { return _readConnection->getReadNotifier(); }

    protected:
        virtual ~PairConnection();

        virtual int64_t read( void* buffer, const uint64_t bytes )
            { return _readConnection.get()->read( buffer, bytes ); }
        virtual int64_t write( const void* buffer, const uint64_t bytes )
            const { return _writeConnection->write( buffer, bytes ); }

    private:
        PairConnection( PairConnection* sibling );

        eqBase::RefPtr<Connection> _readConnection;
        eqBase::RefPtr<Connection> _writeConnection;
        eqBase::RefPtr<PairConnection> _sibling;
    };
}
#endif //EQNET_PAIRCONNECTION_H

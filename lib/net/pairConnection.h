
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
        eqBase::RefPtr<PairConnection> _sibling;
    };
}
#endif //EQNET_PAIRCONNECTION_H

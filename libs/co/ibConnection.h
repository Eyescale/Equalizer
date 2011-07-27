
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#ifndef CO_IBCONNECTION_H
#define CO_IBCONNECTION_H

#include <eq/base/base.h>
#ifdef EQ_INFINIBAND

#include "socketConnection.h"

#include "ibAdapter.h"
#include "ibCompletionQueue.h"
#include "ibInterface.h"

namespace co
{
class IBConnection : public Connection
{
public:
    IBConnection();
    virtual ~IBConnection();
    virtual bool connect();
    virtual void close() { _close(); }
    virtual Notifier getNotifier() const;

    virtual bool listen();
    virtual void acceptNB();
    virtual ConnectionPtr acceptSync();

    virtual void    readNB  (       void* buffer, const uint64_t bytes );
    virtual int64_t readSync( void* buffer, const uint64_t bytes,
                              const bool ignored );
    virtual int64_t write   ( const void* buffer, const uint64_t bytes );

    void incReadInterface();
    void incWriteInterface();
    void addEvent();
    void removeEvent();

private:

    eq::base::Lock   _mutex;
    eq::base::a_int32_t  numRead;
    eq::base::a_int32_t _comptEvent;
    
    IBAdapter          _adapter;

    std::vector< IBCompletionQueue* >  _completionQueues;
    std::vector< IBInterface* >        _interfaces;

    HANDLE             _readEvent;
    ConnectionPtr _socketConnection; 
    uint32_t numWrite;


    // store info connection
    void _setConnection( ConnectionPtr connection )
            { _socketConnection = connection; }

    // code info IB and send it
    bool _writeKeys( const struct IBDest *myDest );

    // receive info IB and decode it
    bool _readKeys ( struct IBDest *remDest );

    // communication exchange protocol for server
    bool _serverExchDest( const struct IBDest *myDest,
                                struct IBDest* remDest );

    // communication exchange protocol for client 
    bool _clientExchDest( const struct IBDest *myDest,
                                struct IBDest *remDest );

    // exchange IB info between client and server on socket connection
    bool _establish( const bool isServer );
    bool _establish( bool isServer, uint32_t index );

    // init my connection IB
    bool _preRegister();

    void _close();
    EQ_TS_VAR( _recvThread );
};
}
#endif //EQ_INFINIBAND

#endif //CO_IBCONNECTION_H 

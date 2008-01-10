
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pairConnection.h"

using namespace eqNet;

PairConnection::PairConnection( eqBase::RefPtr<Connection> readConnection,
                                eqBase::RefPtr<Connection> writeConnection )
        : _readConnection( readConnection )
        , _writeConnection( writeConnection )
{
    _sibling = new PairConnection( this );
    EQASSERT( readConnection->getState() == STATE_CLOSED );
    EQASSERT( writeConnection->getState() == STATE_CLOSED );
}

PairConnection::PairConnection( PairConnection* sibling )
        : _readConnection( sibling->_writeConnection ),
          _writeConnection( sibling->_readConnection ),
          _sibling( sibling )
{
    EQASSERT( _readConnection->getState() == STATE_CLOSED );
    EQASSERT( _writeConnection->getState() == STATE_CLOSED );
}

PairConnection::~PairConnection()
{
    _readConnection = 0;
    _writeConnection = 0;
    _sibling = 0;
}

eqBase::RefPtr<Connection> PairConnection::getSibling()
{
    return eqBase::RefPtr_static_cast<PairConnection, Connection>( _sibling );
}

bool PairConnection::connect()
{
    if( _state != STATE_CLOSED )
        return false;

    _state           = STATE_CONNECTING;
    _sibling->_state = STATE_CONNECTING;

    if( !_readConnection->connect( ))
    {
        _state           = STATE_CLOSED;
        _sibling->_state = STATE_CLOSED;
        return false;
    }

    if( !_writeConnection->connect( ))
    {
        _readConnection->close();
        _state           = STATE_CLOSED;
        _sibling->_state = STATE_CLOSED;
        return false;
    }

    _state           = STATE_CONNECTED;
    _sibling->_state = STATE_CONNECTED;
    return true;
}

void PairConnection::close()
{
    if( _state != STATE_CONNECTED )
        return;

    _state           = STATE_CLOSED;
    _sibling->_state = STATE_CLOSED;

    _readConnection->close();
    _writeConnection->close();
}

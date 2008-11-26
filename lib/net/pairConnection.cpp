
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pairConnection.h"

using namespace std;

namespace eq
{
namespace net
{
PairConnection::PairConnection( ConnectionPtr readConnection,
                                ConnectionPtr writeConnection )
        : _readConnection( readConnection )
        , _writeConnection( writeConnection )
{
    _sibling = new PairConnection( this );
    EQASSERT( readConnection->isClosed( ));
    EQASSERT( writeConnection->isClosed( ));
    EQINFO << "New PairConnection @" << (void*)this << endl;
}

PairConnection::PairConnection( PairConnection* sibling )
        : _readConnection( sibling->_writeConnection ),
          _writeConnection( sibling->_readConnection ),
          _sibling( sibling )
{
    EQASSERT( _readConnection->isClosed( ));
    EQASSERT( _writeConnection->isClosed( ));
    EQINFO << "New PairConnection Sibling @" << (void*)this << endl;
}

PairConnection::~PairConnection()
{
    _readConnection = 0;
    _writeConnection = 0;
    _sibling = 0;
}

ConnectionPtr PairConnection::getSibling()
{
    return _sibling.get();
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

    // clear all pointers to break circular RefPtr dependency to sibling. This
    // prohibits reopening the pair connection!
    _sibling->_readConnection  = 0;
    _sibling->_writeConnection = 0;
    _sibling->_sibling         = 0;
    _readConnection  = 0;
    _writeConnection = 0;
    _sibling         = 0;
}
}
}

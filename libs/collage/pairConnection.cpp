
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "pairConnection.h"

using namespace std;

namespace co
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

    // Break circular RefPtr dependency to sibling. This prohibits reopening the
    // pair connection!
    _sibling->_sibling = 0;

    _readConnection    = 0;
    _writeConnection   = 0;
    _sibling           = 0;
}

}

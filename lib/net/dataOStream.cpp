
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "dataOStream.h"

#include "node.h"
#include "types.h"

using namespace eq::base;

namespace eq
{
namespace net
{

uint64_t DataOStream::_highWaterMark = 4096;

DataOStream::DataOStream()
        : _bufferStart( 0 )
        , _enabled( false )
        , _dataSent( false )
        , _buffered( true )
        , _save( false )
{
}

DataOStream::~DataOStream()
{
    // Can't call disable() from destructor since it uses virtual functions
    EQASSERT( !_enabled );
}

void DataOStream::enable( const NodeVector& receivers )
{
    const bool useMulticast = receivers.size() > 1;
    ConnectionDescriptionVector mcSet;

    for( NodeVector::const_iterator i = receivers.begin(); 
         i != receivers.end(); ++i )
    {
        NodePtr       node       = *i;
        ConnectionPtr connection = useMulticast ? node->getMulticast() : 0;
        
        if( connection.isValid( ))
        {
            ConnectionDescriptionPtr desc = connection->getDescription();
            if( std::find( mcSet.begin(), mcSet.end(), desc ) != mcSet.end( ))
                // already added by another node
                continue;
            mcSet.push_back( desc );
        }
        else
            connection = node->getConnection();
        
        connection->lockSend();
        _connections.push_back( connection );
    }

    enable();
}

void DataOStream::enable( const NodePtr node )
{
    ConnectionPtr connection = node->getConnection();
        
    connection->lockSend();
    _connections.push_back( connection );
    enable();
}

void DataOStream::enable()
{
    EQASSERT( !_enabled );

    _bufferStart = 0;
    _dataSent = false;
    _buffered = true;
    _buffer.setSize( 0 );
    _enabled  = true;
}

void DataOStream::resend( const NodePtr node )
{
    EQASSERT( !_enabled );
    EQASSERT( _connections.empty( ));
    EQASSERT( _save );
    
    ConnectionPtr connection = node->getConnection();        
    connection->lockSend();
    _connections.push_back( connection );

    sendSingle( _buffer.getData(), _buffer.getSize() );

    _connections.clear();
    connection->unlockSend();
}

void DataOStream::disable()
{
    if( !_enabled )
        return;

    if( _dataSent )
    {
        if( !_connections.empty( ))
            sendFooter( _buffer.getData() + _bufferStart, 
                        _buffer.getSize() - _bufferStart );

        _dataSent = true;
    }
    else if( _buffer.getSize() > 0 )
    {
        EQASSERT( _bufferStart == 0 );
        if( !_connections.empty( ))
            sendSingle( _buffer.getData(), _buffer.getSize() );

        _dataSent = true;
    }

    _resetStart();
    _enabled = false;
    _unlockConnections();
}

void DataOStream::_unlockConnections()
{
    for( ConnectionVector::const_iterator i = _connections.begin(); 
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        connection->unlockSend();
    }
    _connections.clear();
}

void DataOStream::enableBuffering()
{
    _buffered = true;
}

void DataOStream::disableBuffering()
{
    if( !_buffered )
        return;

    _buffered = false;
    flush();
}

void DataOStream::enableSave()
{
    EQASSERTINFO( !_enabled || ( !_dataSent && _buffer.getSize() == 0 ),
                  "Can't enable saving after data has been written" );
    _save = true;
}

void DataOStream::disableSave()
{
    EQASSERTINFO( !_enabled || (!_dataSent && _buffer.getSize() == 0 ),
                  "Can't disable saving after data has been written" );
    _save = false;
}

void DataOStream::write( const void* data, uint64_t size )
{
    EQASSERT( _enabled );
    if( _buffered || _save )
        _buffer.append( static_cast< const uint8_t* >( data ), size );

    if( !_buffered )
    {
        _sendBuffer( data, size );
        return;
    }

    if( _buffer.getSize() - _bufferStart > _highWaterMark )
        flush();
}

void DataOStream::writeOnce( const void* data, uint64_t size )
{
    EQASSERT( _enabled );
    EQASSERT( !_dataSent );
    EQASSERT( _bufferStart == 0 );

    if( _save )
        _buffer.append( static_cast< const uint8_t* >( data ), size );

    if( !_connections.empty( ))
        sendSingle( data, size );

    _resetStart();
    _enabled = false;
    _dataSent = true;
    _unlockConnections();
}

void DataOStream::flush()
{
    EQASSERT( _enabled );
    _sendBuffer( _buffer.getData() + _bufferStart, 
                 _buffer.getSize() - _bufferStart );
    _resetStart();
}

void DataOStream::_resetStart()
{
    if( _save )
        _bufferStart = _buffer.getSize();
    else
    {
        _bufferStart = 0;
        _buffer.setSize( 0 );
    }
}

void DataOStream::_sendBuffer( const void* data, const uint64_t size )
{
    EQASSERT( _enabled );
    if( size == 0 )
        return;

    if( !_dataSent )
    {
        if( !_connections.empty( ))
            sendHeader( data, size );
        _dataSent = true;
        return;
    }

    if( !_connections.empty( ))
        sendBuffer( data, size );
}
}
}

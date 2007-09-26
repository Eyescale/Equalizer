
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "dataOStream.h"

#include "node.h"
#include "types.h"

using namespace eqBase;

namespace eqNet
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
    EQASSERT( !_enabled );

    for( NodeVector::const_iterator i = receivers.begin(); 
         i != receivers.end(); ++i )
    {
        RefPtr< Node >       node       = *i;
        RefPtr< Connection > connection = node->getConnection();
        
        connection->lockSend();
        _connections.push_back( connection );
    }

    _dataSent = false;
    _enabled  = true;
}

void DataOStream::enable( const ConnectionVector& receivers )
{
    EQASSERT( !_enabled );

    for( ConnectionVector::const_iterator i = receivers.begin(); 
         i != receivers.end(); ++i )
    {
        RefPtr< Connection > connection = *i;
        
        connection->lockSend();
        _connections.push_back( connection );
    }

    _dataSent = false;
    _enabled  = true;
}

void DataOStream::disable()
{
    if( !_enabled )
        return;

    flush();
    if( _dataSent )
        sendFooter();

    _enabled = false;

    for( ConnectionVector::const_iterator i = _connections.begin(); 
         i != _connections.end(); ++i )
    {
        RefPtr< Connection > connection = *i;
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
    EQASSERTINFO( !_dataSent && _buffer.size == 0,
                  "Can't enable saving after data has been written" );
    _save = true;
}

void DataOStream::disableSave()
{
    EQASSERTINFO( !_dataSent && _buffer.size == 0,
                  "Can't disable saving after data has been written" );
    _save = false;
}

void DataOStream::swapSaveBuffer( eqBase::Buffer& buffer )
{
    EQASSERT( !_enabled );
    EQASSERT( _save );

    _buffer.swap( buffer );
}
 
void DataOStream::write( const void* data, uint64_t size )
{
    EQASSERT( _enabled );
    if( _buffered || _save )
        _buffer.append( data, size );

    if( !_buffered )
    {
        _sendBuffer( data, size );
        return;
    }

    if( _buffer.size - _bufferStart > _highWaterMark )
        flush();
}

void DataOStream::flush()
{
    EQASSERT( _enabled );
    _sendBuffer( _buffer.data + _bufferStart, _buffer.size - _bufferStart );
    
    if( _save )
        _bufferStart = _buffer.size;
    else
    {
        _bufferStart = 0;
        _buffer.size = 0;
    }
}

void DataOStream::_sendBuffer( const void* data, const uint64_t size )
{
    EQASSERT( _enabled );
    if( size == 0 )
        return;

    if( !_dataSent )
    {
        sendHeader();
        _dataSent = true;
    }

    sendBuffer( data, size );
}
}

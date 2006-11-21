
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include "commands.h"
#include "image.h"
#include "object.h"
#include "packets.h"

#include <eq/net/command.h>
#include <eq/net/session.h>
#include <algorithm>

using namespace eq;
using namespace std;

FrameBuffer::FrameBuffer( const void* data, const uint64_t size )
        : Object( eq::Object::TYPE_FRAMEBUFFER ),
          _data( *(Data*)data ) 
{
    EQASSERT( size == sizeof( Data ));
    setInstanceData( &_data, sizeof( Data ));

    registerCommand( CMD_FRAMEBUFFER_TRANSMIT,
           eqNet::CommandFunc<FrameBuffer>( this, &FrameBuffer::_cmdTransmit ));
    registerCommand( CMD_FRAMEBUFFER_READY,
              eqNet::CommandFunc<FrameBuffer>( this, &FrameBuffer::_cmdReady ));
}

FrameBuffer::~FrameBuffer()
{
    flush();
}

FrameBuffer::FormatIndex FrameBuffer::_getIndexForFormat( const Frame::Format
                                                          format )
{
    switch( format )
    {
        case Frame::FORMAT_COLOR: return INDEX_COLOR;
        case Frame::FORMAT_DEPTH: return INDEX_DEPTH;

        default: EQUNIMPLEMENTED;
    }
    return static_cast<FormatIndex>(-1);
}

void FrameBuffer::clear()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _clearImages( (FormatIndex)i );
}

void FrameBuffer::_clearImages( const FormatIndex i )
{
    copy( _images[i].begin(), _images[i].end(), 
          inserter( _imageCache[i], _imageCache[i].end( )));
    _images[i].clear();
}

void FrameBuffer::flush()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _flushImages( (FormatIndex)i );
}

void FrameBuffer::_flushImages( const FormatIndex i )
{
    _clearImages( i );

    for( vector<Image*>::const_iterator iter = _imageCache[i].begin();
         iter != _imageCache[i].end(); ++iter )

        delete *iter;

    _imageCache[i].clear();
}

Image* FrameBuffer::newImage( const Frame::Format format )
{
    const FormatIndex index = _getIndexForFormat( format );
    Image* image;
    if( _imageCache[index].empty( ))
        image = new Image( format );
    else
    {
        image = _imageCache[index].back();
        _imageCache[index].pop_back();
    }
    _images[index].push_back( image );
    return image;
}

void FrameBuffer::startReadback( const Frame& frame )
{
    PixelViewport absPVP = _data.pvp + frame.getOffset();
    if( !absPVP.isValid( ))
        return;

    if( _data.format & Frame::FORMAT_COLOR )
    {
        Image* image = newImage( Frame::FORMAT_COLOR );
        image->startReadback( absPVP );
    }
    if( _data.format & Frame::FORMAT_DEPTH )
    {
        Image* image = newImage( Frame::FORMAT_DEPTH );
        image->startReadback( absPVP );
    }
}

void FrameBuffer::syncReadback()
{
    for( vector<Image*>::const_iterator iter = _images[INDEX_COLOR].begin();
         iter != _images[INDEX_COLOR].end(); ++iter )
    {
        Image* image = *iter;
        image->syncReadback();
    }
    
    for( vector<Image*>::const_iterator iter = _images[INDEX_DEPTH].begin();
         iter != _images[INDEX_DEPTH].end(); ++iter )
    {
        Image* image = *iter;
        image->syncReadback();
    }
    
    setReady();
}

void FrameBuffer::transmit( eqBase::RefPtr<eqNet::Node> toNode )
{
    FrameBufferTransmitPacket packet;
    const eqNet::Session*     session = getSession();
    EQASSERT( session );

    packet.sessionID = session->getID();
    packet.objectID  = getID();

    _transmit( toNode, packet, Frame::FORMAT_COLOR );
    _transmit( toNode, packet, Frame::FORMAT_DEPTH );

    FrameBufferReadyPacket readyPacket;
    readyPacket.sessionID = session->getID();
    readyPacket.objectID  = getID();
    readyPacket.version   = getVersion();
    toNode->send( readyPacket );
}

void FrameBuffer::_transmit( eqBase::RefPtr<eqNet::Node> toNode, 
                             FrameBufferTransmitPacket& packet, 
                             const Frame::Format format )
{
    if( !(_data.format & format))
        return;

    const uint32_t index = _getIndexForFormat( format );
    for( vector<Image*>::const_iterator iter = _images[index].begin();
         iter != _images[index].end(); ++iter )
    {
        const Image*           image = *iter;
        const vector<uint8_t>& data  = image->getPixelData();

        packet.format = format;
        packet.pvp    = image->getPixelViewport();

        EQASSERT( data.size() == packet.pvp.w*packet.pvp.h*image->getDepth());
        toNode->send<uint8_t>( packet, data );
    }
}

eqNet::CommandResult FrameBuffer::_cmdTransmit( eqNet::Command& command )
{
    const FrameBufferTransmitPacket* packet = 
        command.getPacket<FrameBufferTransmitPacket>();
    // TODO create and attach Image
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult FrameBuffer::_cmdReady( eqNet::Command& command )
{
    const FrameBufferReadyPacket* packet = 
        command.getPacket<FrameBufferReadyPacket>();

    _readyVersion = packet->version;
    return eqNet::COMMAND_HANDLED;
}

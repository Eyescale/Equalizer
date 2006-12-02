
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

#include "commands.h"
#include "image.h"
#include "log.h"
#include "object.h"
#include "packets.h"

#include <eq/base/compressor.h>
#include <eq/net/command.h>
#include <eq/net/session.h>
#include <algorithm>

using namespace eq;
using namespace eqBase;
using namespace std;

FrameData::FrameData( const void* data, const uint64_t size )
        : Object( eq::Object::TYPE_FRAMEDATA ),
          _data( *(Data*)data ) 
{
    EQASSERT( size == sizeof( Data ));
    setInstanceData( &_data, sizeof( Data ));

    registerCommand( CMD_FRAMEDATA_TRANSMIT,
           eqNet::CommandFunc<FrameData>( this, &FrameData::_cmdTransmit ));
    registerCommand( CMD_FRAMEDATA_READY,
              eqNet::CommandFunc<FrameData>( this, &FrameData::_cmdReady ));
}

FrameData::~FrameData()
{
    flush();
}

void FrameData::unpack( const void* data, const uint64_t size )
{ 
    _clear();
    eqNet::Object::unpack( data, size ); 
    getLocalNode()->flushCommands(); // process rescheduled transmit packets
}

void FrameData::_clear()
{
    copy( _images.begin(), _images.end(), 
          inserter( _imageCache, _imageCache.end( )));
    _images.clear();
}

void FrameData::flush()
{
    _clear();

    for( vector<Image*>::const_iterator iter = _imageCache.begin();
         iter != _imageCache.end(); ++iter )

        delete *iter;

    _imageCache.clear();
}

Image* FrameData::newImage()
{
    Image* image;
    if( _imageCache.empty( ))
        image = new Image();
    else
    {
        image = _imageCache.back();
        _imageCache.pop_back();
    }
    _images.push_back( image );
    return image;
}

void FrameData::startReadback( const Frame& frame )
{
    if( _data.buffers == Frame::BUFFER_NONE )
        return;

    PixelViewport absPVP = _data.pvp + frame.getOffset();
    if( !absPVP.isValid( ))
        return;
    
    Image* image = newImage();
    image->startReadback( absPVP, _data.buffers );
}

void FrameData::syncReadback()
{
    for( vector<Image*>::const_iterator iter = _images.begin();
         iter != _images.end(); ++iter )
    {
        Image* image = *iter;
        image->syncReadback();

#if 0
        static uint32_t counter = 0;
        ostringstream stringstream;
        stringstream << "Image_" << ++counter;
        image->writeImages( stringstream.str( ));
#endif
    }
    _setReady();
}

void FrameData::startAssemble( const Frame& frame )
{
    EQLOG( LOG_ASSEMBLY ) << "Assemble " << _images.size() <<" images, buffers "
                          << _data.buffers << " offset " << _data.offset <<endl;
    if( _data.buffers == Frame::BUFFER_NONE )
        return;

    if( _images.empty( ))
        EQWARN << "No Images to assemble" << endl;

    for( vector<Image*>::const_iterator i = _images.begin();
         i != _images.end(); ++i )
    {
        Image* image = *i;
        image->startAssemble( _data.offset, _data.buffers );
    }
}

void FrameData::syncAssemble()
{
    for( vector<Image*>::const_iterator iter = _images.begin();
         iter != _images.end(); ++iter )
    {
        Image* image = *iter;
        image->syncAssemble();
    }
}

void FrameData::transmit( eqBase::RefPtr<eqNet::Node> toNode )
{
    if( _data.buffers == 0 )
    {
        EQWARN << "No buffers for frame data" << endl;
        return;
    }

    FrameDataTransmitPacket packet;
    const uint64_t          packetSize = sizeof( packet ) - 8*sizeof( uint8_t );
    const eqNet::Session*   session    = getSession();
    EQASSERT( session );

    packet.sessionID = session->getID();
    packet.objectID  = getID();
    packet.version   = getVersion();

    for( vector<Image*>::const_iterator iter = _images.begin();
         iter != _images.end(); ++iter )
    {
        const Image*                     image = *iter;
#ifdef EQ_USE_COMPRESSION
        vector<uint8_t>                  compressedData;
#else
        vector< const vector<uint8_t>* > pixelDatas;
#endif

        packet.size    = packetSize;
        packet.buffers = Frame::BUFFER_NONE;
        packet.pvp     = image->getPixelViewport();

        EQASSERT( packet.pvp.isValid( ));

        if( image->hasPixelData( Frame::BUFFER_COLOR ))
        {
            const vector<uint8_t>& data = 
                image->getPixelData( Frame::BUFFER_COLOR );
#ifdef EQ_USE_COMPRESSION
            Compressor::compressLZ( data, compressedData );
#else
            pixelDatas.push_back( &data );
            packet.size    += data.size();
#endif
            packet.buffers |= Frame::BUFFER_COLOR;
        }
        if( image->hasPixelData( Frame::BUFFER_DEPTH ))
        {
            const vector<uint8_t>& data = 
                image->getPixelData( Frame::BUFFER_DEPTH );
#ifdef EQ_USE_COMPRESSION
            Compressor::compressLZ( data, compressedData );
#else
            pixelDatas.push_back( &data );
            packet.size    += data.size();
#endif
            packet.buffers |= Frame::BUFFER_DEPTH;
        }
 
#ifdef EQ_USE_COMPRESSION
       if( compressedData.empty( ))
            continue;

       packet.size += compressedData.size();
#else
       if( pixelDatas.empty( ))
            continue;
#endif

        RefPtr<eqNet::Connection> connection = toNode->getConnection();
        connection->lockSend();
        connection->send( &packet, packetSize, true );

#ifdef EQ_USE_COMPRESSION
        connection->send( &compressedData[0], compressedData.size(), true );
#else
        for( vector< const vector<uint8_t>* >::iterator i = pixelDatas.begin();
             i != pixelDatas.end(); ++i )
        {
            const std::vector<uint8_t>& item = **i;
            connection->send( &item[0], item.size(), true );
        }    
#endif
        connection->unlockSend(); 
    }

    FrameDataReadyPacket readyPacket;
    readyPacket.sessionID = session->getID();
    readyPacket.objectID  = getID();
    readyPacket.version   = getVersion();
    toNode->send( readyPacket );
}

//----- Command handlers

eqNet::CommandResult FrameData::_cmdTransmit( eqNet::Command& command )
{
    const FrameDataTransmitPacket* packet = 
        command.getPacket<FrameDataTransmitPacket>();

    if( getVersion() < packet->version )
        return eqNet::COMMAND_REDISPATCH;

    EQLOG( LOG_ASSEMBLY ) << "received image, buffers " << packet->buffers 
                          << " pvp " << packet->pvp << endl;

    EQASSERT( packet->pvp.isValid( ));
    EQASSERT( getVersion() == packet->version );

    Image*         image   = newImage();
    const uint8_t* data    = packet->data;
    const uint64_t nPixels = packet->pvp.w * packet->pvp.h;
#ifdef EQ_USE_COMPRESSION
    vector<uint8_t> decompressedData;
#endif

    image->setPixelViewport( packet->pvp );

    if( packet->buffers & Frame::BUFFER_COLOR )
    {
#ifdef EQ_USE_COMPRESSION
        data += Compressor::decompressLZ( data, decompressedData );
        EQASSERT( decompressedData.size() == 
                  nPixels * image->getDepth( Frame::BUFFER_COLOR ));

        image->setPixelData( Frame::BUFFER_COLOR, &decompressedData[0] ); 
        decompressedData.clear();
#else
        image->setPixelData( Frame::BUFFER_COLOR, data );
        data += nPixels * image->getDepth( Frame::BUFFER_COLOR );
#endif
    }
    if( packet->buffers & Frame::BUFFER_DEPTH )
    {
#ifdef EQ_USE_COMPRESSION
        data += Compressor::decompressLZ( data, decompressedData );
        EQASSERT( decompressedData.size() == 
                  nPixels * image->getDepth( Frame::BUFFER_DEPTH ));

        image->setPixelData( Frame::BUFFER_DEPTH, &decompressedData[0] ); 
        decompressedData.clear();
#else
        image->setPixelData( Frame::BUFFER_DEPTH, data );
        data += nPixels * image->getDepth( Frame::BUFFER_DEPTH );
#endif
    }

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult FrameData::_cmdReady( eqNet::Command& command )
{
    const FrameDataReadyPacket* packet = 
        command.getPacket<FrameDataReadyPacket>();

    if( getVersion() < packet->version )
        return eqNet::COMMAND_REDISPATCH;

    EQASSERT( getVersion() == packet->version );
    EQLOG( LOG_ASSEMBLY ) << this << " received v" << packet->version
                          << " ready " << _images.size() << " images" << endl;
    
    _readyVersion = packet->version;
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eq::operator << ( std::ostream& os, const FrameData* data )
{
    os << "frame data id " << data->getID() << " v" << data->getVersion()
       << " ready " << data->isReady();
    return os;
}


/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

#include "commands.h"
#include "image.h"
#include "log.h"
#include "object.h"
#include "packets.h"

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
    Image* image = _allocImage();
    _images.push_back( image );
    return image;
}

Image* FrameData::_allocImage()
{
    Image* image;
    if( _imageCache.empty( ))
        image = new Image();
    else
    {
        image = _imageCache.back();
        _imageCache.pop_back();
    }
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
        Image*                   image = *iter;
        vector< const uint8_t* > pixelDatas;
        vector< uint32_t >       pixelDataSizes;

        packet.size    = packetSize;
        packet.buffers = Frame::BUFFER_NONE;
        packet.pvp     = image->getPixelViewport();

        EQASSERT( packet.pvp.isValid( ));

#ifdef EQ_USE_COMPRESSION
        Clock clock;
#endif
        if( image->hasPixelData( Frame::BUFFER_COLOR ))
        {
#ifdef EQ_USE_COMPRESSION
            uint32_t size;
            const uint8_t* data = image->compressPixelData( Frame::BUFFER_COLOR,
                                                            size );
#else
            const uint8_t* data = image->getPixelData( Frame::BUFFER_COLOR );
            const uint32_t size = image->getPixelDataSize( Frame::BUFFER_COLOR);
#endif

            pixelDatas.push_back( data );
            pixelDataSizes.push_back( size );
            packet.size    += size;
            packet.buffers |= Frame::BUFFER_COLOR;
        }
        if( image->hasPixelData( Frame::BUFFER_DEPTH ))
        {
#ifdef EQ_USE_COMPRESSION
            uint32_t size;
            const uint8_t* data = image->compressPixelData( Frame::BUFFER_DEPTH,
                                                            size );
#else
            const uint8_t* data = image->getPixelData( Frame::BUFFER_DEPTH );
            const uint32_t size = image->getPixelDataSize( Frame::BUFFER_DEPTH);
#endif

            pixelDatas.push_back( data );
            pixelDataSizes.push_back( size );
            packet.size    += size;
            packet.buffers |= Frame::BUFFER_DEPTH;
        }
#ifdef EQ_USE_COMPRESSION
        EQLOG( LOG_ASSEMBLY ) << "Image compression took " << clock.getTimef() 
                              << endl;
#endif
 
        if( pixelDatas.empty( ))
            continue;

        RefPtr<eqNet::Connection> connection = toNode->getConnection();
        connection->lockSend();
        connection->send( &packet, packetSize, true );
        
        for( uint32_t i=0; i<pixelDatas.size(); ++i )
            connection->send( pixelDatas[i], pixelDataSizes[i], true );

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

    EQLOG( LOG_ASSEMBLY ) 
        << "received image, buffers " << packet->buffers << " pvp " 
        << packet->pvp << " v" << packet->version << endl;

    EQASSERT( packet->pvp.isValid( ));

    Image*         image   = _allocImage();
    const uint8_t* data    = packet->data;

    image->setPixelViewport( packet->pvp );

#ifdef EQ_USE_COMPRESSION
    Clock          clock;
#endif
    if( packet->buffers & Frame::BUFFER_COLOR )
    {
#ifdef EQ_USE_COMPRESSION
        data += image->decompressPixelData( Frame::BUFFER_COLOR, data );
#else
        image->setPixelData( Frame::BUFFER_COLOR, data );
        data += image->getPixelDataSize( Frame::BUFFER_COLOR );
#endif
    }
    if( packet->buffers & Frame::BUFFER_DEPTH )
    {
#ifdef EQ_USE_COMPRESSION
        data += image->decompressPixelData( Frame::BUFFER_DEPTH, data );
#else
        image->setPixelData( Frame::BUFFER_DEPTH, data );
        data += image->getPixelDataSize( Frame::BUFFER_DEPTH );
#endif
    }
#ifdef EQ_USE_COMPRESSION
    EQLOG( LOG_ASSEMBLY ) << "Image decompression took " << clock.getTimef() 
                          << endl;
#endif
    
    const uint32_t version = getVersion();

    if( version == packet->version )
    {
        EQASSERT( getVersion() == packet->version );
        EQASSERT( _readyVersion.get() < getVersion( ));
        _images.push_back( image );
    }
    else
    {
        EQASSERT( getVersion() < packet->version );
        _pendingImages.push_back( ImageVersion( image, packet->version ));
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
    EQASSERT( _readyVersion.get() < getVersion( ));

    for( list<ImageVersion>::iterator i = _pendingImages.begin();
         i != _pendingImages.end(); )
    {
        const ImageVersion& imageVersion = *i;
        EQASSERT( imageVersion.version >= packet->version );

        if( imageVersion.version == packet->version )
        {
            _images.push_back( imageVersion.image );
            list<ImageVersion>::iterator eraseIter = i;
            ++i;
            _pendingImages.erase( eraseIter );
        }
        else
            ++i;
    }

    EQLOG( LOG_ASSEMBLY ) << this << " received v" << packet->version
                          << " with " << _images.size() << " images" << endl;

    _readyVersion = packet->version;
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eq::operator << ( std::ostream& os, const FrameData* data )
{
    os << "frame data id " << data->getID() << " v" << data->getVersion()
       << " ready " << data->isReady();
    return os;
}


/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

#include "commands.h"
#include "image.h"
#include "log.h"
#include "packets.h"

#include <eq/net/command.h>
#include <eq/net/commandFunc.h>
#include <eq/net/session.h>
#include <algorithm>

using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;

namespace eq
{
FrameData::FrameData()
{
    setInstanceData( &_data, sizeof( Data ));
}

FrameData::~FrameData()
{
    flush();
}

void FrameData::attachToSession( const uint32_t id, const uint32_t instanceID, 
                                 net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_FRAMEDATA_TRANSMIT,
                     CommandFunc<FrameData>( this, &FrameData::_cmdTransmit ),
                     queue );
    registerCommand( CMD_FRAMEDATA_READY,
                     CommandFunc<FrameData>( this, &FrameData::_cmdReady ),
                     queue );
    registerCommand( CMD_FRAMEDATA_UPDATE,
                     CommandFunc<FrameData>( this, &FrameData::_cmdUpdate ),
                     queue );
}

void FrameData::applyInstanceData( net::DataIStream& is )
{ 
    clear();
    net::Object::applyInstanceData( is ); 

    EQLOG( LOG_ASSEMBLY ) << "applied " << this << endl;

    FrameDataUpdatePacket packet; // trigger process of received ready packets
    packet.instanceID = getInstanceID();
    packet.version    = getVersion() + 1;
    send( getLocalNode(), packet );
}

void FrameData::clear()
{
    EQASSERT( _listeners.empty( ));

    _imageCacheLock.set();
    _imageCache.insert( _imageCache.end(), _images.begin(), _images.end( ));
    _imageCacheLock.unset();

    _images.clear();
}

void FrameData::flush()
{
    clear();

    for( vector<Image*>::const_iterator i = _imageCache.begin();
         i != _imageCache.end(); ++i )

        delete *i;

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
    _imageCacheLock.set();

    if( _imageCache.empty( ))
        image = new Image();
    else
    {
        image = _imageCache.back();
        image->reset();
        _imageCache.pop_back();
    }

    _imageCacheLock.unset();
    return image;
}

void FrameData::startReadback( const Frame& frame, 
                               Window::ObjectManager* glObjects )
{
    if( _data.buffers == Frame::BUFFER_NONE )
        return;

    PixelViewport absPVP = _data.pvp + frame.getOffset();
    if( !absPVP.isValid( ))
        return;
    
    Image* image = newImage();
    image->startReadback( _data.buffers, absPVP, glObjects );
}

void FrameData::syncReadback()
{
    for( vector<Image*>::const_iterator iter = _images.begin();
         iter != _images.end(); ++iter )
    {
        Image* image = *iter;
        image->syncReadback();

#ifndef NDEBUG
        if( getenv( "EQ_DUMP_IMAGES" ))
        {
            static uint32_t counter = 0;
            ostringstream stringstream;
            stringstream << "Image_" << ++counter;
            image->writeImages( stringstream.str( ));
        }
#endif
    }
    _setReady( getVersion( ));
}

void FrameData::_setReady( const uint32_t version )
{ 
    EQASSERT( getVersion() == net::Object::VERSION_NONE || 
              _readyVersion < version );

    _listenersMutex.set();
#ifndef NDEBUG
    for( list<ImageVersion>::iterator i = _pendingImages.begin();
         i != _pendingImages.end(); ++i )
    {
        const ImageVersion& imageVersion = *i;
        EQASSERTINFO( imageVersion.version > version, 
                      "Frame is ready, but not all images have been set" );
    }
#endif

    _readyVersion = version; 
    EQLOG( LOG_ASSEMBLY ) << "set ready " << this << endl;

    for( vector< Monitor<uint32_t>* >::iterator i = _listeners.begin();
         i != _listeners.end(); ++i )
    {
        Monitor<uint32_t>* monitor = *i;
        ++(*monitor);
    }

    _listenersMutex.unset();
}

void FrameData::transmit( net::NodePtr toNode )
{
    if( _data.buffers == 0 )
    {
        EQWARN << "No buffers for frame data" << endl;
        return;
    }

    FrameDataTransmitPacket packet;
    const uint64_t          packetSize = sizeof( packet ) - 8*sizeof( uint8_t );
    const net::Session*   session    = getSession();
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
            EQLOG( LOG_ASSEMBLY )
                << "Compress color "
                << image->getPixelDataSize( Frame::BUFFER_COLOR ) << "->" 
                << size << " done at " << clock.getTimef() << endl;
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
            EQLOG( LOG_ASSEMBLY )
                << "Compress depth "
                << image->getPixelDataSize( Frame::BUFFER_DEPTH ) << "->" 
                << size << " done at " << clock.getTimef() << endl;
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
        EQLOG( LOG_ASSEMBLY ) << "Image compress took " << clock.getTimef() 
                              << endl;
        clock.reset();
#endif
 
        if( pixelDatas.empty( ))
            continue;

        RefPtr<net::Connection> connection = toNode->getConnection();
        connection->lockSend();
        connection->send( &packet, packetSize, true );
        
        for( uint32_t i=0; i<pixelDatas.size(); ++i )
            connection->send( pixelDatas[i], pixelDataSizes[i], true );

        connection->unlockSend(); 
#ifdef EQ_USE_COMPRESSION
        EQLOG( LOG_ASSEMBLY ) << "Image transmit took " << clock.getTimef() 
                              << endl;
        clock.reset();
#endif
    }

    FrameDataReadyPacket readyPacket;
    readyPacket.sessionID = session->getID();
    readyPacket.objectID  = getID();
    readyPacket.version   = getVersion();
    toNode->send( readyPacket );
}

void FrameData::addListener( eq::base::Monitor<uint32_t>& listener )
{
    _listenersMutex.set();

    _listeners.push_back( &listener );
    if( _readyVersion >= getVersion( ))
        ++listener;

    _listenersMutex.unset();
}

void FrameData::removeListener( eq::base::Monitor<uint32_t>& listener )
{
    _listenersMutex.set();

    vector< Monitor<uint32_t>* >::iterator i = find( _listeners.begin(),
                                                     _listeners.end(),
                                                     &listener );
    EQASSERT( i != _listeners.end( ));
    _listeners.erase( i );

    _listenersMutex.unset();
}

//----- Command handlers

net::CommandResult FrameData::_cmdTransmit( net::Command& command )
{
    CHECK_THREAD( _commandThread );
    const FrameDataTransmitPacket* packet = 
        command.getPacket<FrameDataTransmitPacket>();

    EQLOG( LOG_ASSEMBLY ) 
        << this << " received image, buffers " << packet->buffers << " pvp " 
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
        EQLOG( LOG_ASSEMBLY ) << "Decompress color done at " 
                              << clock.getTimef() << endl;
#else
        image->setPixelData( Frame::BUFFER_COLOR, data );
        data += image->getPixelDataSize( Frame::BUFFER_COLOR );
#endif
    }

    if( packet->buffers & Frame::BUFFER_DEPTH )
    {
#ifdef EQ_USE_COMPRESSION
        data += image->decompressPixelData( Frame::BUFFER_DEPTH, data );
        EQLOG( LOG_ASSEMBLY ) << "Decompress depth done at " 
                              << clock.getTimef() << endl;
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
        EQASSERT( _readyVersion < getVersion( ));
        _images.push_back( image );
    }
    else
    {
        EQASSERT( version < packet->version );
        _pendingImages.push_back( ImageVersion( image, packet->version ));
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult FrameData::_cmdReady( net::Command& command )
{
    CHECK_THREAD( _commandThread );
    const FrameDataReadyPacket* packet = 
        command.getPacket<FrameDataReadyPacket>();

    if( getVersion() == packet->version )
    {
        _applyVersion( packet->version );
        _setReady( packet->version );
    }
    else
        _readyVersions.insert( packet->version );

    EQLOG( LOG_ASSEMBLY ) << this << " received v" << packet->version << endl;

    return net::COMMAND_HANDLED;
}

net::CommandResult FrameData::_cmdUpdate( net::Command& command )
{
    CHECK_THREAD( _commandThread );
    const FrameDataUpdatePacket* packet = 
        command.getPacket<FrameDataUpdatePacket>();

    _applyVersion( packet->version );

    std::set< uint32_t >::iterator i = _readyVersions.find( packet->version );
    if( i != _readyVersions.end( ))
    {
        _readyVersions.erase( i );
        _setReady( packet->version );
    }

    return net::COMMAND_HANDLED;
}

void FrameData::_applyVersion( const uint32_t version )
{
    CHECK_THREAD( _commandThread );
    EQLOG( LOG_ASSEMBLY ) << this << " apply v" << version << endl;

    if( _readyVersion == version )
    {
#ifndef NDEBUG
        for( list<ImageVersion>::iterator i = _pendingImages.begin();
             i != _pendingImages.end(); ++i )
        {
            const ImageVersion& imageVersion = *i;
            EQASSERTINFO( imageVersion.version > version, 
                          "Frame is ready, but not all images have been set" );
        }
#endif

        // already applied
        return;
    }

    EQASSERTINFO( _readyVersion < version, "old " << _readyVersion.get()
                  << " new " << version << " for " << this );

    for( list<ImageVersion>::iterator i = _pendingImages.begin();
         i != _pendingImages.end(); )
    {
        const ImageVersion& imageVersion = *i;
        EQASSERT( imageVersion.version >= version );

        if( imageVersion.version == version )
        {
            _images.push_back( imageVersion.image );
            list<ImageVersion>::iterator eraseIter = i;
            ++i;
            _pendingImages.erase( eraseIter );
        }
        else
            ++i;
    }

    EQLOG( LOG_ASSEMBLY ) << this << " applied v" << version << endl;
}

std::ostream& operator << ( std::ostream& os, const FrameData* data )
{
    os << "frame data id " << data->getID() << "." << data->getInstanceID() 
       << " v" << data->getVersion() << ' ' << data->getImages().size() 
       << " images, ready " << ( data->isReady() ? 'y' :'n' );
    return os;
}

}

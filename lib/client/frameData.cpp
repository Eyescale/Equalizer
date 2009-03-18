
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "frameData.h"

#include "commands.h"
#include "image.h"
#include "log.h"
#include "packets.h"
#include "roiFinder.h"

#include <eq/net/command.h>
#include <eq/net/commandFunc.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/net/session.h>
#include <eq/base/monitor.h>
#include <algorithm>


using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;

namespace eq
{
FrameData::FrameData()
{
    _roiFinder = new ROIFinder();
    EQINFO << "New FrameData @" << (void*)this << endl;
}

FrameData::~FrameData()
{
    clear();

    for( vector<Image*>::const_iterator i = _imageCache.begin();
         i != _imageCache.end(); ++i )
    {
        Image* image = *i;
        EQWARN << "Unflushed image in FrameData destructor" << endl;
        delete image;
    }
    _imageCache.clear();

    delete _roiFinder;
    _roiFinder = 0;
}

void FrameData::getInstanceData( net::DataOStream& os )
{
    EQUNREACHABLE;
    os << _data;
}

void FrameData::applyInstanceData( net::DataIStream& is )
{
    clear();
    is >> _data;
    EQLOG( LOG_ASSEMBLY ) << "applied " << this << endl;
}

void FrameData::update( const uint32_t version )
{
    // trigger process of received ready packets
    FrameDataUpdatePacket packet;
    packet.instanceID = getInstanceID();
    packet.version    = version;
    send( getLocalNode(), packet );
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
    {
        Image* image = *i;
        image->flush();
        delete image;
    }

    _imageCache.clear();
}

Image* FrameData::newImage( const eq::Frame::Type type )
{
    Image* image = _allocImage( type );
    _images.push_back( image );
    return image;
}

Image* FrameData::_allocImage( const eq::Frame::Type type )
{
    Image* image;
    _imageCacheLock.set();

    if( _imageCache.empty( ))
    {
        _imageCacheLock.unset();
        image = new Image;

        if( type == Frame::TYPE_TEXTURE )
        {
            image->setFormat( Frame::BUFFER_COLOR, GL_RGBA );
            image->setType(   Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
        }
        else
        {
            image->setFormat( Frame::BUFFER_COLOR, GL_BGRA );
            image->setType(   Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
        }
        image->setFormat( Frame::BUFFER_DEPTH, GL_DEPTH_COMPONENT );
        image->setType(   Frame::BUFFER_DEPTH, GL_UNSIGNED_INT );
    }
    else
    {
        image = _imageCache.back();
        _imageCache.pop_back();
        _imageCacheLock.unset();

        image->reset();
    }

    image->setStorageType( type );
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

    const Zoom& zoom = frame.getZoom();

    if( !zoom.isValid( ))
    {
        EQWARN << "Invalid zoom factor, skipping frame." << endl;
        return;
    }

    PixelViewportVector pvps;

    if( _data.buffers & Frame::BUFFER_DEPTH && zoom == Zoom::NONE )
        pvps = _roiFinder->findRegions( _data.buffers, absPVP, zoom, glObjects);
    else
        pvps.push_back( absPVP );

    for( uint32_t i = 0; i < pvps.size(); i++ )
    {
        PixelViewport pvp = pvps[ i ];
        pvp.intersect( absPVP );
                
        Image* image = newImage( _data.frameType );
        image->startReadback( _data.buffers, pvp, zoom, glObjects );
        image->setOffset( pvp.x - absPVP.x, pvp.y - absPVP.y );
    }
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
    setReady();
}

void FrameData::setReady()
{
    _setReady( getVersion( ));
}

void FrameData::_setReady( const uint32_t version )
{
    EQASSERTINFO( getVersion() == net::Object::VERSION_NONE || 
                  _readyVersion <= version,
                  "v" << getVersion() << " ready " << _readyVersion << " new "
                      << version );

    base::ScopedMutex< base::Lock > mutex( _listenersMutex );
#ifndef NDEBUG
    for( list<ImageVersion>::iterator i = _pendingImages.begin();
         i != _pendingImages.end(); ++i )
    {
        const ImageVersion& imageVersion = *i;
        EQASSERTINFO( imageVersion.version > version,
                      "Frame is ready, but not all images have been set" );
    }
#endif

    if( _readyVersion >= version )
        return;

    _readyVersion = version;
    EQLOG( LOG_ASSEMBLY ) << "set ready " << this << ", " << _listeners.size()
                          << " monitoring" << endl;

    for( vector< Monitor<uint32_t>* >::iterator i = _listeners.begin();
         i != _listeners.end(); ++i )
    {
        Monitor<uint32_t>* monitor = *i;
        ++(*monitor);
    }
}

namespace
{
typedef Image::PixelData::Chunk Chunk;
}

int64_t FrameData::transmit( net::NodePtr toNode )
{
    if( _data.buffers == 0 )
    {
        EQWARN << "No buffers for frame data" << endl;
        return 0;
    }

    if ( _data.frameType == Frame::TYPE_TEXTURE )
    {
        EQWARN << "Can't transmit image of type TEXTURE" << endl;
        EQUNIMPLEMENTED;
        return 0;
    }

    net::ConnectionPtr             connection = toNode->getConnection();
    net::ConnectionDescriptionPtr description = connection->getDescription();

    // use compression on links up to 2 GBit/s
    const bool useCompression = ( description->bandwidth <= 262144 );
    float      compressTime   = 0.f;

    FrameDataTransmitPacket packet;
    const uint64_t          packetSize = sizeof( packet ) - 8*sizeof( uint8_t );
    const net::Session*     session    = getSession();
    EQASSERT( session );

    packet.sessionID    = session->getID();
    packet.objectID     = getID();
    packet.version      = getVersion();
    packet.isCompressed = useCompression;

    // send all images
    for( vector<Image*>::const_iterator i = _images.begin(); 
         i != _images.end(); ++i )
    {
        Image* image = *i;
        vector< const Image::PixelData* > pixelDatas;

        packet.size    = packetSize;
        packet.buffers = Frame::BUFFER_NONE;
        packet.pvp     = image->getPixelViewport();

        EQASSERT( packet.pvp.isValid( ));

        // Prepare image pixel data
        Frame::Buffer buffers[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
        for( unsigned j = 0; j < 2; ++j )
        {
            Frame::Buffer buffer = buffers[j];
            if( image->hasPixelData( buffer ))
            {
                packet.size += 3 * sizeof( uint32_t ); // format, type, nChunks

                if( useCompression )
                {
                    Clock clock;
                    const Image::PixelData& data =
                        image->compressPixelData( buffer );
                    compressTime += clock.getTimef();

                    pixelDatas.push_back( &data );

                    for( vector< Chunk* >::const_iterator k =
                             data.chunks.begin(); k != data.chunks.end(); ++k )
                    {
                        const Chunk* chunk = *k;
                        packet.size += chunk->headerSize;
                        packet.size += chunk->size;
                    }
                }
                else
                {
                    const Image::PixelData& data = image->getPixelData( buffer);
                    pixelDatas.push_back( &data );

                    packet.size += data.chunks[0]->headerSize;
                    packet.size += data.chunks[0]->size;
                }

                packet.buffers |= buffer;
            }
        }

        if( pixelDatas.empty( ))
            continue;
        
        // send image pixel data packet
#ifdef EQ_SEND_TOKEN
        getLocalNode()->acquireSendToken( toNode );
#endif
        connection->lockSend();
        connection->send( &packet, packetSize, true );
#ifndef NDEBUG
        size_t sentBytes = packetSize;
#endif

        for( uint32_t j=0; j < pixelDatas.size(); ++j )
        {
            const Image::PixelData* data = pixelDatas[j];
            uint32_t imageHeader[3] = { data->format, data->type, 
                                        data->chunks.size() };
            connection->send( imageHeader, 3 * sizeof( uint32_t ), true );
#ifndef NDEBUG
            sentBytes += 3 * sizeof( uint32_t );
#endif

            for( vector< Chunk* >::const_iterator k = 
                     data->chunks.begin(); k != data->chunks.end(); ++k )
            {
                const Chunk* chunk = *k;
                connection->send( chunk, chunk->headerSize + chunk->size, true);
#ifndef NDEBUG
                sentBytes += chunk->headerSize + chunk->size;
#endif
            }
        }
#ifndef NDEBUG
        EQASSERTINFO( sentBytes == packet.size,
                      sentBytes << " != " << packet.size );
#endif

        connection->unlockSend();
#ifdef EQ_SEND_TOKEN
        getLocalNode()->releaseSendToken( toNode );
#endif
    }

    FrameDataReadyPacket readyPacket;
    readyPacket.sessionID = session->getID();
    readyPacket.objectID  = getID();
    readyPacket.version   = getVersion();
    toNode->send( readyPacket );
    return static_cast< int64_t >( compressTime );
}

void FrameData::addListener( base::Monitor<uint32_t>& listener )
{
    _listenersMutex.set();

    _listeners.push_back( &listener );
    if( _readyVersion >= getVersion( ))
        ++listener;

    _listenersMutex.unset();
}

void FrameData::removeListener( base::Monitor<uint32_t>& listener )
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

    Image*   image = _allocImage( Frame::TYPE_MEMORY );
    // Note on the const_cast: since the PixelData structure stores non-const
    // pointers, we have to go non-const at some point, even though we do not
    // modify the data.
    uint8_t* data  = const_cast< uint8_t* >( packet->data );

    image->setPixelViewport( packet->pvp );

    Frame::Buffer buffers[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
    for( unsigned i = 0; i < 2; ++i )
    {
        Frame::Buffer buffer = buffers[i];
        
        if( packet->buffers & buffer )
        {
            Image::PixelData pixelData;
            const uint32_t*  u32Data   = reinterpret_cast< uint32_t* >( data );
            
            pixelData.format     = u32Data[0];
            pixelData.type       = u32Data[1];
            pixelData.compressed = packet->isCompressed;

            const uint32_t nChunks = u32Data[2];
            
            data += 3 * sizeof( uint32_t );
            
            for( uint32_t j=0; j < nChunks; ++j )
            {
                Chunk* chunk = reinterpret_cast< Chunk* >( data );

                pixelData.chunks.push_back( chunk );
                data += chunk->headerSize + chunk->size;
            }

            image->setPixelData( buffer, pixelData );

            // Prevent ~PixelData from freeing pointers
            pixelData.chunks.clear();
        }
    }

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

    // Input images sync() to the new version, then send an update packet and
    // immediately continue. If they read back and setReady faster than we
    // process this update packet, the readyVersion changes at any given point
    // in this code.
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

    // Even if _readyVersion jumped to version in between, there are no pending
    // images for it, so this loop doesn't do anything.
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

            EQASSERT( _readyVersion < version );
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

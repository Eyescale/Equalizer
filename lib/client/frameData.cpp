
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "frameData.h"

#include "config.h"
#include "frameDataStatistics.h"
#include "image.h"
#include "log.h"
#include "node.h"
#include "packets.h"
#include "roiFinder.h"

#include <eq/net/command.h>
#include <eq/net/commandFunc.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/net/session.h>
#include <eq/base/monitor.h>

#include <eq/plugins/compressor.h>
#include <algorithm>

namespace eq
{

namespace
{
struct ImageHeader
{
    uint32_t                internalFormat;
    uint32_t                externalFormat;
    uint32_t                pixelSize;
    fabric::PixelViewport   pvp;
    uint32_t                compressorName;
    uint32_t                compressorFlags;
    uint32_t                nChunks;
    bool                    hasAlpha;
    bool                    fill[7];
};

}

typedef net::CommandFunc<FrameData> CmdFunc;

FrameData::FrameData() 
        : _useAlpha( true )
        , _useSendToken( false )
        , _colorQuality( 1.f )
        , _depthQuality( 1.f )
{
    _roiFinder = new ROIFinder();
    EQINFO << "New FrameData @" << (void*)this << std::endl;
}

FrameData::~FrameData()
{
    clear();

    for( Images::const_iterator i = _imageCache.begin();
         i != _imageCache.end(); ++i )
    {
        Image* image = *i;
        EQWARN << "Unflushed image in FrameData destructor" << std::endl;
        delete image;
    }
    _imageCache.clear();

    delete _roiFinder;
    _roiFinder = 0;
}

void FrameData::setQuality( Frame::Buffer buffer, float quality )
{
    if( buffer != Frame::BUFFER_COLOR )
    {
        EQASSERT( buffer == Frame::BUFFER_DEPTH );
        _depthQuality = quality;
        return;
    }

    _colorQuality = quality;
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
    EQLOG( LOG_ASSEMBLY ) << "applied " << this << std::endl;
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

    registerCommand( fabric::CMD_FRAMEDATA_TRANSMIT,
                     CmdFunc( this, &FrameData::_cmdTransmit ), queue );
    registerCommand( fabric::CMD_FRAMEDATA_READY,
                     CmdFunc( this, &FrameData::_cmdReady ), queue );
    registerCommand( fabric::CMD_FRAMEDATA_UPDATE,
                     CmdFunc( this, &FrameData::_cmdUpdate ), queue );
}

void FrameData::clear()
{
    EQASSERT( _listeners->empty( ));

    _imageCacheLock.set();
    _imageCache.insert( _imageCache.end(), _images.begin(), _images.end( ));
    _imageCacheLock.unset();

    _images.clear();
}

void FrameData::flush()
{
    clear();

    for( Images::const_iterator i = _imageCache.begin();
         i != _imageCache.end(); ++i )
    {
        Image* image = *i;
        image->flush();
        delete image;
    }

    _imageCache.clear();
}

Image* FrameData::newImage( const eq::Frame::Type type,
                            const DrawableConfig& config )
{
    Image* image = _allocImage( type, config );
    _images.push_back( image );
    return image;
}

Image* FrameData::_allocImage( const eq::Frame::Type type,
                               const DrawableConfig& config )
{
    Image* image;
    _imageCacheLock.set();

    if( _imageCache.empty( ))
    {
        _imageCacheLock.unset();
        image = new Image;
    }
    else
    {
        image = _imageCache.back();
        _imageCache.pop_back();
        _imageCacheLock.unset();

        image->reset();
    }

    _useAlpha ? image->enableAlphaUsage() : image->disableAlphaUsage();

    image->setStorageType( type );
    image->setQuality( Frame::BUFFER_COLOR, _colorQuality );
    image->setQuality( Frame::BUFFER_DEPTH, _depthQuality ); 
    image->setInternalFormat( Frame::BUFFER_DEPTH,
                              EQ_COMPRESSOR_DATATYPE_DEPTH );

    switch( config.colorBits )
    {
        case 16:  
            image->setInternalFormat( Frame::BUFFER_COLOR, 
                                      EQ_COMPRESSOR_DATATYPE_RGBA16F );
            break;
        case 32:  
            image->setInternalFormat( Frame::BUFFER_COLOR, 
                                      EQ_COMPRESSOR_DATATYPE_RGBA32F );
            break;
        case 10:
            image->setInternalFormat( Frame::BUFFER_COLOR, 
                                      EQ_COMPRESSOR_DATATYPE_RGB10_A2 );
            break;
        default:
            image->setInternalFormat( Frame::BUFFER_COLOR, 
                                      EQ_COMPRESSOR_DATATYPE_RGBA );
    }

    return image;
}


void FrameData::readback( const Frame& frame,
                          util::ObjectManager< const void* >* glObjects,
                          const DrawableConfig& config  )
{
    if( _data.buffers == Frame::BUFFER_NONE )
        return;

    PixelViewport absPVP = _data.pvp + frame.getOffset();
    if( !absPVP.isValid( ))
        return;

    const Zoom& zoom = frame.getZoom();

    if( !zoom.isValid( ))
    {
        EQWARN << "Invalid zoom factor, skipping frame" << std::endl;
        return;
    }

    PixelViewports pvps;

    if( _data.buffers & Frame::BUFFER_DEPTH && zoom == Zoom::NONE )
        pvps = _roiFinder->findRegions( _data.buffers, absPVP, zoom,
//                    frame.getAssemblyStage(), frame.getFrameID(), glObjects );
                    0, 0, glObjects );
    else
        pvps.push_back( absPVP );

    for( uint32_t i = 0; i < pvps.size(); i++ )
    {
        PixelViewport pvp = pvps[ i ];
        pvp.intersect( absPVP );
                
        Image* image = newImage( _data.frameType, config );
        image->readback( _data.buffers, pvp, zoom, glObjects );
        image->setOffset( pvp.x - absPVP.x, pvp.y - absPVP.y );

#ifndef NDEBUG
        if( getenv( "EQ_DUMP_IMAGES" ))
        {
            static base::a_int32_t counter;
            std::ostringstream stringstream;

            stringstream << "Image_" << std::setfill( '0' ) << std::setw(5)
                         << ++counter;
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
    EQASSERTINFO( getVersion() == net::VERSION_NONE || _readyVersion <= version,
                  "v" << getVersion() << " ready " << _readyVersion << " new "
                      << version );

    base::ScopedMutex< base::SpinLock > mutex( _listeners );
#ifndef NDEBUG
    for( std::list< ImageVersion >::iterator i = _pendingImages.begin();
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
    EQLOG( LOG_ASSEMBLY ) << "set ready " << this << ", " << _listeners->size()
                          << " monitoring" << std::endl;

    for( Monitors::iterator i=_listeners->begin(); i != _listeners->end(); ++i )
    {
        Monitor* monitor = *i;
        ++(*monitor);
    }
}


void FrameData::transmit( net::NodePtr toNode, const uint32_t frameNumber,
                          const uint32_t originator )
{
    FrameDataStatistics event( Statistic::FRAME_TRANSMIT, this, frameNumber,
                               originator );

    if( _data.buffers == 0 )
    {
        EQWARN << "No buffers for frame data" << std::endl;
        return;
    }

    if ( _data.frameType == Frame::TYPE_TEXTURE )
    {
        EQWARN << "Can't transmit image of type TEXTURE" << std::endl;
        EQUNIMPLEMENTED;
        return;
    }

    net::ConnectionPtr             connection = toNode->getConnection();
    net::ConnectionDescriptionPtr description = connection->getDescription();

    // use compression on links up to 2 GBit/s
    const bool useCompression = ( description->bandwidth <= 262144 );

    FrameDataTransmitPacket packet;
    const uint64_t          packetSize = sizeof( packet ) - 8*sizeof( uint8_t );
    const net::Session*     session    = getSession();
    EQASSERT( session );

    packet.sessionID    = session->getID();
    packet.objectID     = getID();
    packet.version      = getVersion();
    packet.frameNumber  = frameNumber;

    // send all images
    for( Images::const_iterator i = _images.begin(); i != _images.end(); ++i )
    {
        Image* image = *i;
        std::vector< const Image::PixelData* > pixelDatas;

        packet.size           = packetSize;
        packet.buffers        = Frame::BUFFER_NONE;
        packet.pvp            = image->getPixelViewport();
        packet.ignoreAlpha    = image->ignoreAlpha();

        EQASSERT( packet.pvp.isValid( ));

        {
            uint64_t rawSize( 0 );
            FrameDataStatistics compressEvent( Statistic::FRAME_COMPRESS, this, 
                                               frameNumber, originator );
            compressEvent.event.data.statistic.ratio = 1.0f;
            if( !useCompression ) // don't send event
                compressEvent.event.data.statistic.frameNumber = 0;

            // Prepare image pixel data
            Frame::Buffer buffers[] = {Frame::BUFFER_COLOR,Frame::BUFFER_DEPTH};

            // for each image attachment
            for( unsigned j = 0; j < 2; ++j )
            {
                Frame::Buffer buffer = buffers[j];
                if( image->hasPixelData( buffer ))
                {
                    // format, type, nChunks, compressor name
                    packet.size += sizeof( ImageHeader ); 

                    const Image::PixelData& data = useCompression ?
                        image->compressPixelData( buffer ) : 
                        image->getPixelData( buffer );
                    pixelDatas.push_back( &data );
                    
                    if( data.isCompressed )
                    {
                        const uint32_t nElements = data.compressedSize.size();
                        for( uint32_t k = 0 ; k < nElements; ++k )
                        {
                            packet.size += sizeof( uint64_t );
                            packet.size += data.compressedSize[ k ];
                        }
                    }
                    else
                    {
                        packet.size += sizeof( uint64_t );
                        packet.size += image->getPixelDataSize( buffer );
                    }

                    packet.buffers |= buffer;
                    rawSize += image->getPixelDataSize( buffer );
                }
            }

            if( rawSize > 0 )
                compressEvent.event.data.statistic.ratio =
                    static_cast< float >( packet.size ) /
                    static_cast< float >( rawSize );
        }

        if( pixelDatas.empty( ))
            continue;

        // send image pixel data packet
        if( _useSendToken )
            getLocalNode()->acquireSendToken( toNode );

        connection->lockSend();
        connection->send( &packet, packetSize, true );
#ifndef NDEBUG
        size_t sentBytes = packetSize;
#endif

        for( uint32_t j=0; j < pixelDatas.size(); ++j )
        {
#ifndef NDEBUG
            sentBytes += sizeof( ImageHeader );
#endif
            const Image::PixelData* data = pixelDatas[j];
            const ImageHeader imageHeader =
                  { data->internalFormat, data->externalFormat,
                    data->pixelSize, data->pvp,
                    data->compressorName, data->compressorFlags,
                    data->isCompressed ? data->compressedSize.size() : 1,
                    data->hasAlpha, { false }};

            connection->send( &imageHeader, sizeof( ImageHeader ), true );
            
            if( data->isCompressed )
            {
                for( uint32_t k = 0 ; k < data->compressedSize.size(); ++k )
                {
                    const uint64_t dataSize = data->compressedSize[k];
                    connection->send( &dataSize, sizeof( dataSize ), true );
                    if( dataSize > 0 )
                        connection->send( data->compressedData[k], 
                                          dataSize, true );
#ifndef NDEBUG
                    sentBytes += sizeof( dataSize ) + dataSize;
#endif
                }
            }
            else
            {
                const uint64_t dataSize = data->pvp.getArea() * 
                                          data->pixelSize;
                connection->send( &dataSize, sizeof( dataSize ), true );
                connection->send( data->pixels, dataSize, true );
#ifndef NDEBUG
                sentBytes += sizeof( dataSize ) + dataSize;
#endif
            }
        }
#ifndef NDEBUG
        EQASSERTINFO( sentBytes == packet.size,
                      sentBytes << " != " << packet.size );
#endif

        connection->unlockSend();
        if( _useSendToken )
            getLocalNode()->releaseSendToken( toNode );
    }

    FrameDataReadyPacket readyPacket;
    readyPacket.zoom      = _zoom;
    readyPacket.sessionID = session->getID();
    readyPacket.objectID  = getID();
    readyPacket.version   = getVersion();
    toNode->send( readyPacket );
}

void FrameData::addListener( base::Monitor<uint32_t>& listener )
{
    base::ScopedMutex< base::SpinLock > mutex( _listeners );

    _listeners->push_back( &listener );
    if( _readyVersion >= getVersion( ))
        ++listener;
}

void FrameData::removeListener( base::Monitor<uint32_t>& listener )
{
    base::ScopedMutex< base::SpinLock > mutex( _listeners );

    Monitors::iterator i = std::find( _listeners->begin(), _listeners->end(),
                                      &listener );
    EQASSERT( i != _listeners->end( ));
    _listeners->erase( i );
}

//----- Command handlers

bool FrameData::_cmdTransmit( net::Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const FrameDataTransmitPacket* packet =
        command.getPacket<FrameDataTransmitPacket>();

    EQLOG( LOG_ASSEMBLY )
        << this << " received image, buffers " << packet->buffers << " pvp "
        << packet->pvp << " v" << packet->version << std::endl;

    EQASSERT( packet->pvp.isValid( ));

    // Very ugly way to get our local eq::Node object identifier
    uint32_t originator = getID();
    Config* config = EQSAFECAST( Config*, getSession( ));
    if( config )
        originator = config->getNodes().front()->getID();

    FrameDataStatistics event( Statistic::FRAME_RECEIVE, this, 
                               packet->frameNumber, originator );

    Image*   image = _allocImage( Frame::TYPE_MEMORY, DrawableConfig( ));
    // Note on the const_cast: since the PixelData structure stores non-const
    // pointers, we have to go non-const at some point, even though we do not
    // modify the data.
    uint8_t* data  = const_cast< uint8_t* >( packet->data );

    image->setPixelViewport( packet->pvp );
    packet->ignoreAlpha ? image->disableAlphaUsage() :image->enableAlphaUsage();

    Frame::Buffer buffers[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
    for( unsigned i = 0; i < 2; ++i )
    {
        Frame::Buffer buffer = buffers[i];
        
        if( packet->buffers & buffer )
        {
            Image::PixelData pixelData;
            const ImageHeader* imageHeader = 
                              reinterpret_cast< ImageHeader* >( data );

            pixelData.internalFormat  = imageHeader->internalFormat;
            pixelData.externalFormat  = imageHeader->externalFormat;
            pixelData.pixelSize       = imageHeader->pixelSize;
            pixelData.pvp             = imageHeader->pvp;
            pixelData.compressorName  = imageHeader->compressorName;
            pixelData.compressorFlags = imageHeader->compressorFlags;
            pixelData.hasAlpha        = imageHeader->hasAlpha;
            pixelData.isCompressed = 
                pixelData.compressorName > EQ_COMPRESSOR_NONE;

            const uint32_t nChunks    = imageHeader->nChunks;
            data += sizeof( ImageHeader );

            if( pixelData.isCompressed )
            {
                pixelData.compressedSize.resize( nChunks );
                pixelData.compressedData.resize( nChunks );

                for( uint32_t j = 0; j < nChunks; ++j )
                {
                    const uint64_t size = *reinterpret_cast< uint64_t*>( data );
                    data += sizeof( uint64_t );
                    
                    pixelData.compressedSize[j] = size; 
                    pixelData.compressedData[j] = data;
                    data += size;
                }
            }
            else
            {
                const uint64_t size = *reinterpret_cast< uint64_t*>( data );
                data += sizeof( uint64_t );
                pixelData.pixels = data;
                data += pixelData.pvp.getArea() * pixelData.pixelSize;
            }
            image->setPixelData( buffer, pixelData );
            // Prevent ~PixelData from freeing pointers
            pixelData.compressedSize.clear();
            pixelData.compressedData.clear();
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

    return true;
}

bool FrameData::_cmdReady( net::Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const FrameDataReadyPacket* packet =
        command.getPacket<FrameDataReadyPacket>();

    if( getVersion() == packet->version )
    {
        _applyVersion( packet->version );
        _zoom = packet->zoom;
        _setReady( packet->version );
    }
    else
    {
        command.retain();
        _readyVersions.push_back( &command );
    }

    EQLOG( LOG_ASSEMBLY ) << this << " received v" << packet->version << std::endl;
    return true;
}

bool FrameData::_cmdUpdate( net::Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const FrameDataUpdatePacket* packet =
        command.getPacket<FrameDataUpdatePacket>();

    _applyVersion( packet->version );

    for( Commands::iterator i = _readyVersions.begin();
         i != _readyVersions.end(); ++i )
    {
        net::Command* cmd = *i;
        const FrameDataReadyPacket* candidate =
            cmd->getPacket<FrameDataReadyPacket>();

        if( candidate->version != packet->version )
            continue;

        _zoom = candidate->zoom;
        cmd->release();
        _readyVersions.erase( i );
        _setReady( packet->version );
        break;        
    }

    return true;
}

void FrameData::_applyVersion( const uint32_t version )
{
    EQ_TS_THREAD( _commandThread );
    EQLOG( LOG_ASSEMBLY ) << this << " apply v" << version << std::endl;

    // Input images sync() to the new version, then send an update packet and
    // immediately continue. If they read back and setReady faster than we
    // process this update packet, the readyVersion changes at any given point
    // in this code.
    if( _readyVersion == version )
    {
#ifndef NDEBUG
        for( std::list< ImageVersion >::iterator i = _pendingImages.begin();
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
    for( std::list< ImageVersion >::iterator i = _pendingImages.begin();
         i != _pendingImages.end(); )
    {
        const ImageVersion& imageVersion = *i;
        EQASSERT( imageVersion.version >= version );

        if( imageVersion.version == version )
        {
            _images.push_back( imageVersion.image );
            i = _pendingImages.erase( i );

            EQASSERT( _readyVersion < version );
        }
        else
            ++i;
    }

    EQLOG( LOG_ASSEMBLY ) << this << " applied v" << version << std::endl;
}

std::ostream& operator << ( std::ostream& os, const FrameData* data )
{
    os << "frame data id " << data->getID() << "." << data->getInstanceID()
       << " v" << data->getVersion() << ' ' << data->getImages().size()
       << " images, ready " << ( data->isReady() ? 'y' :'n' );
    return os;
}

}


/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "nodeStatistics.h"
#include "channelStatistics.h"
#include "exception.h"
#include "image.h"
#include "log.h"
#include "pixelData.h"
#include "roiFinder.h"

#include <eq/fabric/drawableConfig.h>
#include <eq/util/objectManager.h>
#include <co/commandFunc.h>
#include <co/connectionDescription.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <lunchbox/monitor.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/plugins/compressor.h>

#include <boost/foreach.hpp>

#include <algorithm>

namespace eq
{
typedef lunchbox::Monitor< uint64_t > Monitor;
typedef std::vector< FrameData::Listener* > Listeners;

namespace detail
{
class FrameData
{
public:
    FrameData()
        : version( co::VERSION_NONE.low( ))
        , useAlpha( true )
        , colorQuality( 1.f )
        , depthQuality( 1.f )
        , colorCompressor( EQ_COMPRESSOR_AUTO )
        , depthCompressor( EQ_COMPRESSOR_AUTO )
    {}

    eq::FrameData::Data data;

    Images images;
    Images imageCache;
    lunchbox::Lock imageCacheLock;

    ROIFinder roiFinder;

    Images pendingImages;

    uint64_t version; //!< The current version

    /** Data ready monitor for output->input synchronization. */
    Monitor readyVersion;

    /** External monitors for readiness synchronization. */
    lunchbox::Lockable< Listeners, lunchbox::SpinLock > listeners;

    bool useAlpha;
    float colorQuality;
    float depthQuality;

    uint32_t colorCompressor;
    uint32_t depthCompressor;
};
}

typedef co::CommandFunc<FrameData> CmdFunc;

FrameData::FrameData()
    : _impl( new detail::FrameData )
{}

FrameData::~FrameData()
{
    clear();

    BOOST_FOREACH( Image* image, _impl->imageCache )
    {
        LBWARN << "Unflushed image in FrameData destructor" << std::endl;
        delete image;
    }
    _impl->imageCache.clear();

    delete _impl;
}


Frame::Type FrameData::getType() const
{
    return _impl->data.frameType;
}

void FrameData::setType( const Frame::Type type )
{
    _impl->data.frameType = type;
}

uint32_t FrameData::getBuffers() const
{
    return _impl->data.buffers;
}

void FrameData::setBuffers( const uint32_t buffers )
{
    _impl->data.buffers = buffers;
}

const Range& FrameData::getRange() const
{
    return _impl->data.range;
}

void FrameData::setRange( const Range& range )
{
    _impl->data.range = range;
}

const Pixel& FrameData::getPixel() const
{
    return _impl->data.pixel;
}

const SubPixel& FrameData::getSubPixel() const
{
    return _impl->data.subpixel;
}

uint32_t FrameData::getPeriod() const
{
    return _impl->data.period;
}

uint32_t FrameData::getPhase() const
{
    return _impl->data.phase;
}

const Images& FrameData::getImages() const
{
    return _impl->images;
}

void FrameData::setPixelViewport( const PixelViewport& pvp )
{
    _impl->data.pvp = pvp;
}

const PixelViewport& FrameData::getPixelViewport() const
{
    return _impl->data.pvp;
}

void FrameData::setAlphaUsage( const bool useAlpha )
{
    _impl->useAlpha = useAlpha;
}

void FrameData::setZoom( const Zoom& zoom )
{
    _impl->data.zoom = zoom;
}

const Zoom& FrameData::getZoom() const
{
    return _impl->data.zoom;
}

bool FrameData::isReady() const
{
    return _impl->readyVersion.get() >= _impl->version;
}

void FrameData::disableBuffer( const Frame::Buffer buffer )
{
    _impl->data.buffers &= ~buffer;
}

const FrameData::Data& FrameData::getData() const
{
    return _impl->data;
}

void FrameData::setQuality( Frame::Buffer buffer, float quality )
{
    if( buffer != Frame::BUFFER_COLOR )
    {
        LBASSERT( buffer == Frame::BUFFER_DEPTH );
        _impl->depthQuality = quality;
        return;
    }

    _impl->colorQuality = quality;
}

void FrameData::useCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    if( buffer != Frame::BUFFER_COLOR )
    {
        LBASSERT( buffer == Frame::BUFFER_DEPTH );
        _impl->depthCompressor = name;
        return;
    }

    _impl->colorCompressor = name;
}

void FrameData::getInstanceData( co::DataOStream& os )
{
    LBUNREACHABLE;
    _impl->data.serialize( os );
}

void FrameData::applyInstanceData( co::DataIStream& is )
{
    clear();
    _impl->data.deserialize( is );
    LBLOG( LOG_ASSEMBLY ) << "applied " << this << std::endl;
}

void FrameData::Data::serialize( co::DataOStream& os ) const
{
    os << pvp << frameType << buffers << period << phase << range
       << pixel << subpixel << zoom;
}

void FrameData::Data::deserialize( co::DataIStream& is )
{
    is >> pvp >> frameType >> buffers >> period >> phase >> range
       >> pixel >> subpixel >> zoom;
}

void FrameData::clear()
{
    _impl->imageCacheLock.set();
    _impl->imageCache.insert( _impl->imageCache.end(), _impl->images.begin(), _impl->images.end( ));
    _impl->imageCacheLock.unset();
    _impl->images.clear();
}

void FrameData::flush()
{
    clear();

    for( ImagesCIter i = _impl->imageCache.begin(); i != _impl->imageCache.end(); ++i )
    {
        Image* image = *i;
        image->flush();
        delete image;
    }

    _impl->imageCache.clear();
}

void FrameData::deleteGLObjects( util::ObjectManager& om )
{
    for( ImagesCIter i = _impl->images.begin(); i != _impl->images.end(); ++i )
        (*i)->deleteGLObjects( om );
    for( ImagesCIter i = _impl->imageCache.begin(); i != _impl->imageCache.end(); ++i )
        (*i)->deleteGLObjects( om );
}

void FrameData::resetPlugins()
{
    for( ImagesCIter i = _impl->images.begin(); i != _impl->images.end(); ++i )
        (*i)->resetPlugins();
    for( ImagesCIter i = _impl->imageCache.begin(); i != _impl->imageCache.end(); ++i )
        (*i)->resetPlugins();
}

Image* FrameData::newImage( const eq::Frame::Type type,
                            const DrawableConfig& config )
{
    Image* image = _allocImage( type, config, true /* set quality */ );
    _impl->images.push_back( image );
    return image;
}

Image* FrameData::_allocImage( const eq::Frame::Type type,
                               const DrawableConfig& config,
                               const bool setQuality_ )
{
    Image* image;
    _impl->imageCacheLock.set();

    if( _impl->imageCache.empty( ))
    {
        _impl->imageCacheLock.unset();
        image = new Image;
    }
    else
    {
        image = _impl->imageCache.back();
        _impl->imageCache.pop_back();
        _impl->imageCacheLock.unset();

        image->reset();
    }

    image->setAlphaUsage( _impl->useAlpha );
    image->setStorageType( type );
    if( setQuality_ )
    {
        image->setQuality( Frame::BUFFER_COLOR, _impl->colorQuality );
        image->setQuality( Frame::BUFFER_DEPTH, _impl->depthQuality );
    }

    image->useCompressor( Frame::BUFFER_COLOR, _impl->colorCompressor );
    image->useCompressor( Frame::BUFFER_DEPTH, _impl->depthCompressor );

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

#ifndef EQ_2_0_API
void FrameData::readback( const Frame& frame,
                          util::ObjectManager& glObjects,
                          const DrawableConfig& config )
{
    const Images& images = startReadback( frame, glObjects, config,
                                        PixelViewports( 1, getPixelViewport( )));

    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( frame.getZoom(), glObjects.glewGetContext( ));
}
#endif

Images FrameData::startReadback( const Frame& frame,
                                 util::ObjectManager& glObjects,
                                 const DrawableConfig& config,
                                 const PixelViewports& regions )
{
    if( _impl->data.buffers == Frame::BUFFER_NONE )
        return Images();

    const Zoom& zoom = frame.getZoom();
    if( !zoom.isValid( ))
    {
        LBWARN << "Invalid zoom factor, skipping frame" << std::endl;
        return Images();
    }

    const eq::PixelViewport& framePVP = getPixelViewport();
    const PixelViewport      absPVP   = framePVP + frame.getOffset();
    if( !absPVP.isValid( ))
        return Images();

    Images images;

    // readback the whole screen when using textures
    if( getType() == eq::Frame::TYPE_TEXTURE )
    {
        Image* image = newImage( getType(), config );
        if( image->startReadback( getBuffers(), absPVP, zoom, glObjects ))
            images.push_back( image );
        image->setOffset( 0, 0 );
        return images;
    }
    //else read only required regions

#if 0
    // TODO: issue #85: move automatic ROI detection to eq::Channel
    PixelViewports regions;
    if( _impl->data.buffers & Frame::BUFFER_DEPTH && zoom == Zoom::NONE )
        regions = _impl->roiFinder->findRegions( _impl->data.buffers, absPVP,
                                                 zoom, frame.getAssemblyStage(),
                                                 frame.getFrameID(), glObjects);
    else
        regions.push_back( absPVP );
#endif

    LBASSERT( getType() == eq::Frame::TYPE_MEMORY );
    const eq::Pixel& pixel = getPixel();

    for( uint32_t i = 0; i < regions.size(); ++i )
    {
        PixelViewport pvp = regions[ i ] + frame.getOffset();
        pvp.intersect( absPVP );
        if( !pvp.hasArea( ))
            continue;

        Image* image = newImage( getType(), config );
        if( image->startReadback( getBuffers(), pvp, zoom, glObjects ))
            images.push_back( image );

        pvp -= frame.getOffset();
        pvp.apply( zoom );
        image->setOffset( (pvp.x - framePVP.x) * pixel.w,
                          (pvp.y - framePVP.y) * pixel.h );
    }
    return images;
}

void FrameData::setVersion( const uint64_t version )
{
    LBASSERTINFO( _impl->version <= version, _impl->version << " > " << version );
    _impl->version = version;
    LBLOG( LOG_ASSEMBLY ) << "New v" << version << std::endl;
}

void FrameData::waitReady( const uint32_t timeout ) const
{
    if( !_impl->readyVersion.timedWaitGE( _impl->version, timeout ))
        throw Exception( Exception::TIMEOUT_INPUTFRAME );
}

void FrameData::setReady()
{
    _setReady( _impl->version );
}

void FrameData::setReady( const co::ObjectVersion& frameData,
                          const FrameData::Data& data )
{
    clear();
    LBASSERT(  frameData.version.high() == 0 );
    LBASSERT( _impl->readyVersion < frameData.version.low( ));
    LBASSERT( _impl->readyVersion == 0 ||
              _impl->readyVersion + 1 == frameData.version.low( ));
    LBASSERT( _impl->version == frameData.version.low( ));

    _impl->images.swap( _impl->pendingImages );
    _impl->data = data;
    _setReady( frameData.version.low());

    LBLOG( LOG_ASSEMBLY ) << this << " applied v"
                          << frameData.version.low() << std::endl;
}

void FrameData::_setReady( const uint64_t version )
{

    LBASSERTINFO( _impl->readyVersion <= version,
                  "v" << _impl->version << " ready " << _impl->readyVersion << " new "
                      << version );

    lunchbox::ScopedMutex< lunchbox::SpinLock > mutex( _impl->listeners );
    if( _impl->readyVersion >= version )
        return;

    _impl->readyVersion = version;
    LBLOG( LOG_ASSEMBLY ) << "set ready " << this << ", "
                          << _impl->listeners->size() << " monitoring"
                          << std::endl;

    BOOST_FOREACH( Listener* listener, _impl->listeners.data )
        ++(*listener);
}

void FrameData::addListener( Listener& listener )
{
    lunchbox::ScopedFastWrite mutex( _impl->listeners );

    _impl->listeners->push_back( &listener );
    if( _impl->readyVersion >= _impl->version )
        ++listener;
}

void FrameData::removeListener( Listener& listener )
{
    lunchbox::ScopedFastWrite mutex( _impl->listeners );

    Listeners::iterator i = lunchbox::find( _impl->listeners.data, &listener );
    LBASSERT( i != _impl->listeners->end( ));
    _impl->listeners->erase( i );
}

bool FrameData::addImage( const co::ObjectVersion& frameDataVersion,
                          const PixelViewport& pvp, const Zoom& zoom,
                          const uint32_t buffers_, const bool useAlpha,
                          uint8_t* data )
{
    LBASSERT( _impl->readyVersion < frameDataVersion.version.low( ));
    if( _impl->readyVersion >= frameDataVersion.version.low( ))
        return false;

    Image* image = _allocImage( Frame::TYPE_MEMORY, DrawableConfig(),
                                false /* set quality */ );

    image->setPixelViewport( pvp );
    image->setAlphaUsage( useAlpha );

    Frame::Buffer buffers[] = { Frame::BUFFER_COLOR, Frame::BUFFER_DEPTH };
    for( unsigned i = 0; i < 2; ++i )
    {
        const Frame::Buffer buffer = buffers[i];

        if( buffers_ & buffer )
        {
            PixelData pixelData;
            const ImageHeader* header = reinterpret_cast<ImageHeader*>( data );
            pixelData.internalFormat  = header->internalFormat;
            pixelData.externalFormat  = header->externalFormat;
            pixelData.pixelSize       = header->pixelSize;
            pixelData.pvp             = header->pvp;
            pixelData.compressorName  = header->compressorName;
            pixelData.compressorFlags = header->compressorFlags;
            pixelData.isCompressed =
                pixelData.compressorName > EQ_COMPRESSOR_NONE;

            const uint32_t nChunks    = header->nChunks;
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
                data += size;
                LBASSERT( size == pixelData.pvp.getArea()*pixelData.pixelSize );
            }

            image->setZoom( zoom );
            image->setQuality( buffer, header->quality );
            image->setPixelData( buffer, pixelData );
        }
    }

    _impl->pendingImages.push_back( image );
    return true;
}

std::ostream& operator << ( std::ostream& os, const FrameData& data )
{
    return os << "frame data id " << data.getID() << "." << data.getInstanceID()
              << " v" << data.getVersion() << ' ' << data.getImages().size()
              << " images, ready " << ( data.isReady() ? 'y' :'n' ) << " "
              << data.getZoom();
}

}


/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "channel.h"

#include "config.h"
#include "configEvent.h"

#ifdef WIN32_API
#  define snprintf _snprintf
#endif

namespace eqPixelBench
{
namespace
{
#pragma warning(disable: 411) // class defines no constructor to initialize ...
struct EnumMap
{
    const char*    internalFormatString;
    const uint32_t internalFormat;
    const size_t   pixelSize;
};

#pragma warning(default: 411)

#define ENUM_MAP_ITEM( internalFormat, pixelSize )  \
    { #internalFormat, EQ_COMPRESSOR_DATATYPE_ ## internalFormat, pixelSize }

static EnumMap _enums[] = {
    ENUM_MAP_ITEM( RGBA32F, 16 ), // initial buffer resize
    ENUM_MAP_ITEM( RGBA, 4 ),
    ENUM_MAP_ITEM( RGB10_A2, 4 ),
    ENUM_MAP_ITEM( RGBA16F, 8 ),
    ENUM_MAP_ITEM( RGBA32F, 16 ),
    ENUM_MAP_ITEM( DEPTH, 4 ),
    { 0, 0, 0 }};
#define NUM_IMAGES 8
}

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
{
    eq::FrameDataPtr frameData = new eq::FrameData;
    _frame.setFrameData( frameData );

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
        frameData->newImage( eq::Frame::TYPE_MEMORY, getDrawableConfig( ));
}

bool Channel::configExit()
{
    _frame.getFrameData()->resetPlugins();
    return eq::Channel::configExit();
}

void Channel::frameStart( const eq::uint128_t& frameID,
                          const uint32_t frameNumber )
{
    Config* config = static_cast< Config* >( getConfig( ));
    const lunchbox::Clock* clock  = config->getClock();

    if( clock )
    {
        const std::string formatType = "app->pipe thread latency";
        _sendEvent( START_LATENCY, clock->getTimef(), eq::Vector2i( 0, 0 ),
                    formatType, 0, 0 );
    }

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameDraw( const eq::uint128_t& )
{
    //----- setup GL state
    applyBuffer();
    applyViewport();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    setupAssemblyState();

    _testFormats( 1.0f );
    _testFormats( 0.5f );
    _testFormats( 2.0f );
    _testTiledOperations();
    _testDepthAssemble();

    resetAssemblyState();
}

void Channel::_testFormats( float applyZoom )
{
    glGetError(); // reset

    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    eq::Image*        image  = images[ 0 ];
    LBASSERT( image );

    const eq::PixelViewport& pvp = getPixelViewport();
    const eq::Vector2i offset( pvp.x, pvp.y );
    const eq::Zoom zoom( applyZoom, applyZoom );

    lunchbox::Clock clock;
    eq::util::ObjectManager& glObjects = getObjectManager();

    //----- test all default format/type combinations
    for( uint32_t i=0; _enums[i].internalFormatString; ++i )
    {
        const uint32_t internalFormat = _enums[i].internalFormat;
        image->flush();
        image->setInternalFormat( eq::Frame::BUFFER_COLOR, internalFormat );
        image->setQuality( eq::Frame::BUFFER_COLOR, 0.f );
        image->setAlphaUsage( false );

        const GLEWContext* glewContext = glewGetContext();
        const std::vector< uint32_t >& names =
            image->findTransferers( eq::Frame::BUFFER_COLOR, glewContext );

        for( std::vector< uint32_t >::const_iterator j = names.begin();
             j != names.end(); ++j )
        {
            _draw( co::uint128_t( ));

            image->allocDownloader( eq::Frame::BUFFER_COLOR, *j, glewContext );
            image->setPixelViewport( pvp );

            const uint32_t outputToken =
                image->getExternalFormat( eq::Frame::BUFFER_COLOR );
            std::stringstream formatType;
            formatType << std::hex << *j << ':'
                       << _enums[i].internalFormatString << '/' << outputToken
                       << std::dec;
            // read
            glFinish();
            size_t nLoops = 0;
            try
            {
                clock.reset();
                while( clock.getTime64() < 100 /*ms*/ )
                {
                    image->startReadback( eq::Frame::BUFFER_COLOR, pvp,
                                          getContext(), zoom, glObjects );
                    image->finishReadback( glObjects.glewGetContext( ));
                    ++nLoops;
                }
                glFinish();
                const float msec = clock.getTimef() / float( nLoops );
                const GLenum error = glGetError(); // release mode
                if( error != GL_NO_ERROR )
                    throw eq::GLException( error );

                const eq::PixelData& pixels =
                    image->getPixelData( eq::Frame::BUFFER_COLOR );
                const eq::Vector2i area( pixels.pvp.w, pixels.pvp.h );
                const uint64_t dataSizeGPU = area.x() * area.y() *
                                             _enums[i].pixelSize;
                const uint64_t dataSizeCPU =
                    image->getPixelDataSize( eq::Frame::BUFFER_COLOR );

                _sendEvent( READBACK, msec, area, formatType.str(), dataSizeGPU,
                            dataSizeCPU );
            }
            catch( const eq::GLException& e )
            {
                _sendEvent( READBACK, -static_cast<float>( e.glError ),
                            eq::Vector2i(), formatType.str(), 0, 0 );
                continue;
            }

            // write
            eq::Compositor::ImageOp op;
            op.channel = this;
            op.buffers = eq::Frame::BUFFER_COLOR;
            op.offset = offset;
            op.zoom = zoom;

            const uint64_t dataSizeCPU =
                image->getPixelDataSize( eq::Frame::BUFFER_COLOR );
            try
            {
                clock.reset();
                eq::Compositor::assembleImage( image, op );
                glFinish();
                const float msec = clock.getTimef() / float( nLoops );
                const GLenum error = glGetError(); // release mode
                if( error != GL_NO_ERROR )
                    throw eq::Exception( error );

                const eq::PixelData& pixels =
                    image->getPixelData( eq::Frame::BUFFER_COLOR );
                const eq::Vector2i area( pixels.pvp.w, pixels.pvp.h );
                const uint64_t dataSizeGPU =
                    image->getPixelDataSize( eq::Frame::BUFFER_COLOR );
                _sendEvent( ASSEMBLE, msec, area, formatType.str(), dataSizeGPU,
                            dataSizeCPU );
            }
            catch( const eq::GLException& e ) // debug mode
            {
                _sendEvent( ASSEMBLE, -static_cast<float>( e.glError ),
                            eq::Vector2i(), formatType.str(), 0, 0 );
            }
        }
    }
}

void Channel::_testTiledOperations()
{
    glGetError(); // reset

    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    LBASSERT( images[0] );

    const eq::PixelViewport& pvp    = getPixelViewport();
    const eq::Vector2i offset( pvp.x, pvp.y );

    eq::Vector2i area;
    area.x() = pvp.w;

    lunchbox::Clock clock;
    eq::util::ObjectManager& glObjects = getObjectManager();
    const GLEWContext* glewContext = glewGetContext();

    //----- test tiled assembly algorithms
    eq::PixelViewport subPVP = pvp;
    subPVP.h /= NUM_IMAGES;

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        LBASSERT( images[ i ] );
        images[ i ]->setPixelViewport( subPVP );
    }

    for( unsigned tiles = 0; tiles < NUM_IMAGES; ++tiles )
    {
        EQ_GL_CALL( _draw( co::uint128_t( )));
        area.y() = subPVP.h * (tiles+1);

        //---- readback of 'tiles' depth images
        std::stringstream formatType;
        formatType << tiles+1 << " depth tiles";

        float msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];
            LBCHECK( image->allocDownloader( eq::Frame::BUFFER_DEPTH,
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                             glewContext ));
            image->clearPixelData( eq::Frame::BUFFER_DEPTH );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_DEPTH, subPVP, getContext(),
                                  eq::Zoom::NONE, glObjects );
            image->finishReadback( glObjects.glewGetContext( ));
            msec += clock.getTimef();
        }

        _sendEvent( READBACK, msec, area, formatType.str(), 0, 0 );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage( images[j],
                            "EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT",
                            "tiles" );

        //---- readback of 'tiles' color images
        formatType.str("");
        formatType << tiles+1 << " color tiles";

        msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];

            LBCHECK( image->allocDownloader( eq::Frame::BUFFER_COLOR,
                                            EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
                                              glewContext ));
            image->clearPixelData( eq::Frame::BUFFER_COLOR );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_COLOR, subPVP, getContext(),
                                  eq::Zoom::NONE, glObjects );
            image->finishReadback( glObjects.glewGetContext( ));
            msec += clock.getTimef();
        }
        _sendEvent( READBACK, msec, area, formatType.str(), 0, 0 );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage( images[j],"EQ_COMPRESSOR_DATATYPE_BGRA","tiles" );

        //---- benchmark assembly operations
        subPVP.y = pvp.y + tiles * subPVP.h;

        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        formatType.str("");
        formatType << "tiles, GL1.1, " << tiles+1 << " images";

        clock.reset();
        for( unsigned j = 0; j <= tiles; ++j )
            eq::Compositor::assembleImage( images[j], op );

        msec = clock.getTimef();
        _sendEvent( ASSEMBLE, msec, area, formatType.str(), 0, 0 );

        // CPU
        formatType.str("");
        formatType << "tiles, CPU,   " << tiles+1 << " images";

        std::vector< eq::Frame* > frames;
        frames.push_back( &_frame );

        clock.reset();
        eq::Compositor::assembleFramesCPU( frames, this );
        msec = clock.getTimef();
        _sendEvent( ASSEMBLE, msec, area, formatType.str(), 0, 0 );
    }
}

void Channel::_testDepthAssemble()
{
    glGetError(); // reset

    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    eq::Image* image  = images[ 0 ];
    LBASSERT( image );

    const eq::PixelViewport& pvp    = getPixelViewport();
    const eq::Vector2i offset( pvp.x, pvp.y );

    eq::Vector2i area;
    area.x() = pvp.w;

    lunchbox::Clock clock;
    eq::util::ObjectManager& glObjects = getObjectManager();
    const GLEWContext* glewContext = glewGetContext();

    //----- test depth-based assembly algorithms
    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        image = images[ i ];
        LBASSERT( image );
        image->setPixelViewport( pvp );
    }

    area.y() = pvp.h;

    for( uint64_t i = 0; i < NUM_IMAGES; ++i )
    {
        _draw( co::uint128_t( i ) );

        // fill depth & color image
        image = images[ i ];

        LBCHECK( image->allocDownloader( eq::Frame::BUFFER_COLOR,
                                         EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
                                         glewContext ));

        LBCHECK( image->allocDownloader( eq::Frame::BUFFER_DEPTH,
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                         glewContext ));

        image->clearPixelData( eq::Frame::BUFFER_COLOR );
        image->clearPixelData( eq::Frame::BUFFER_DEPTH );

        image->startReadback( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH,
                              pvp, getContext(), eq::Zoom::NONE, glObjects );
        image->finishReadback( glObjects.glewGetContext( ));
        LBASSERT( image->hasPixelData( eq::Frame::BUFFER_COLOR ));
        LBASSERT( image->hasPixelData( eq::Frame::BUFFER_DEPTH ));

        if( i == NUM_IMAGES-1 )
            _saveImage( image,"EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT",
                              "depthAssemble" );

        // benchmark
        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        std::stringstream formatType;
        formatType << "depth, GL1.1, " << i+1 << " images";

        clock.reset();
        for( unsigned j = 0; j <= i; ++j )
            eq::Compositor::assembleImageDB_FF( images[j], op );

        float msec = clock.getTimef();
        _sendEvent( ASSEMBLE, msec, area, formatType.str(), 0, 0 );

        // GLSL
        if( GLEW_VERSION_3_3 )
        {
            formatType.str("");
            formatType << "depth, GLSL,  " << i+1 << " images";

            clock.reset();
            for( unsigned j = 0; j <= i; ++j )
                eq::Compositor::assembleImageDB_GLSL( images[j], op );
            msec = clock.getTimef();
            _sendEvent( ASSEMBLE, msec, area, formatType.str(), 0, 0 );
        }

        // CPU
        formatType.str("");
        formatType << "depth, CPU,   " << i+1 << " images";

        std::vector< eq::Frame* > frames;
        frames.push_back( &_frame );

        clock.reset();
        eq::Compositor::assembleFramesCPU( frames, this );
        msec = clock.getTimef();
        _sendEvent( ASSEMBLE, msec, area, formatType.str(), 0, 0 );
    }
}

void Channel::_sendEvent( ConfigEventType type, const float msec,
                          const eq::Vector2i& area,
                          const std::string& formatType,
                          const uint64_t dataSizeGPU,
                          const uint64_t dataSizeCPU )
{
    std::string name = getName();
    if( name.empty( ))
    {
        std::stringstream strName;
        strName << this;
        name = strName.str();
    }

    getConfig()->sendEvent( type )
        << msec << name << area << formatType << dataSizeGPU << dataSizeCPU;
}

void Channel::_saveImage( const eq::Image* image,
                          const char*      externalformat,
                          const char*      info    )
{
    return;

    static uint32_t counter = 0;
    std::ostringstream stringstream;
    stringstream << "Image_" << ++counter << "_" << externalformat << "_"
                 << info;
    image->writeImages( stringstream.str( ));
}

void Channel::_draw( const eq::uint128_t& spin )
{
    bindFrameBuffer();
    EQ_GL_CALL( glPushAttrib( GL_ALL_ATTRIB_BITS ));

    eq::Channel::frameDraw( spin );
    EQ_GL_CALL( glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT ));
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));

    const float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );

    // rotate scene around the origin
    glRotatef( static_cast< float >( spin.low() + 3 ) * 10, 1.0f, 0.5f, 0.25f );

    // render six axis-aligned colored quads around the origin
    //  front
    glColor3f( 1.0f, 0.5f, 0.5f );
    glNormal3f( 0.0f, 0.0f, 1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, -1.0f );
    glVertex3f( -.7f,  .7f, -1.0f );
    glVertex3f(  .7f, -.7f, -1.0f );
    glVertex3f( -.7f, -.7f, -1.0f );
    glEnd();

    //  bottom
    glColor3f( 0.5f, 1.0f, 0.5f );
    glNormal3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, -1.0f,  .7f );
    glVertex3f( -.7f, -1.0f,  .7f );
    glVertex3f(  .7f, -1.0f, -.7f );
    glVertex3f( -.7f, -1.0f, -.7f );
    glEnd();

    //  back
    glColor3f( 0.5f, 0.5f, 1.0f );
    glNormal3f( 0.0f, 0.0f, -1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, 1.0f );
    glVertex3f( -.7f,  .7f, 1.0f );
    glVertex3f(  .7f, -.7f, 1.0f );
    glVertex3f( -.7f, -.7f, 1.0f );
    glEnd();

    //  top
    glColor3f( 1.0f, 1.0f, 0.5f );
    glNormal3f( 0.f, -1.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, 1.0f,  .7f );
    glVertex3f( -.7f, 1.0f,  .7f );
    glVertex3f(  .7f, 1.0f, -.7f );
    glVertex3f( -.7f, 1.0f, -.7f );
    glEnd();

    //  right
    glColor3f( 1.0f, 0.5f, 1.0f );
    glNormal3f( -1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( 1.0f,  .7f,  .7f );
    glVertex3f( 1.0f, -.7f,  .7f );
    glVertex3f( 1.0f,  .7f, -.7f );
    glVertex3f( 1.0f, -.7f, -.7f );
    glEnd();

    //  left
    glColor3f( 0.5f, 1.0f, 1.0f );
    glNormal3f( 1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -1.0f,  .7f,  .7f );
    glVertex3f( -1.0f, -.7f,  .7f );
    glVertex3f( -1.0f,  .7f, -.7f );
    glVertex3f( -1.0f, -.7f, -.7f );
    glEnd();

    EQ_GL_CALL( glPopAttrib( ));
}


}

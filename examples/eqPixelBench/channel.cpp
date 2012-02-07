
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <co/plugins/compressor.h>

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
    eq::FrameData* frameData = new eq::FrameData;
    _frame.setData( frameData );

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
        frameData->newImage( eq::Frame::TYPE_MEMORY, getDrawableConfig( ));
}

void Channel::frameStart( const eq::uint128_t& frameID, const uint32_t frameNumber ) 
{
    Config* config = static_cast< Config* >( getConfig( ));
    const co::base::Clock* clock  = config->getClock();

    if( clock )
    {
        ConfigEvent   event;
        event.msec = clock->getTimef();

        const std::string& name  = getName();
        if( name.empty( ))    
            snprintf( event.data.user.data, 32, "%p", this);
        else
            snprintf( event.data.user.data, 32, "%s", name.c_str( ));

        event.data.user.data[31] = 0;
        event.area.x() = 0;
        event.area.y() = 0;

        snprintf( event.formatType, 32, "app->pipe thread latency");
        event.data.type = ConfigEvent::START_LATENCY;

        config->sendEvent( event );
    }

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameDraw( const eq::uint128_t& frameID )
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

ConfigEvent Channel::_createConfigEvent()
{
    ConfigEvent   event;
    const std::string& name = getName();

    if( name.empty( ))    
        snprintf( event.data.user.data, 32, "%p", this );
    else
        snprintf( event.data.user.data, 32, "%s", name.c_str( ));

    event.data.user.data[31] = 0;
    return event;
}

void Channel::_testFormats( float applyZoom )
{
    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    eq::Image*        image  = images[ 0 ];
    EQASSERT( image );

    Config* config = static_cast< Config* >( getConfig( ));
    const eq::PixelViewport& pvp = getPixelViewport();
    const eq::Vector2i offset( pvp.x, pvp.y );
    const eq::Zoom zoom( applyZoom, applyZoom );

    co::base::Clock clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();

    //----- test all default format/type combinations
    ConfigEvent event = _createConfigEvent();
    glGetError();
    for( uint32_t i=0; _enums[i].internalFormatString; ++i )
    {
        const uint32_t internalFormat = _enums[i].internalFormat;
        image->flush();
        image->setInternalFormat( eq::Frame::BUFFER_COLOR, internalFormat );
        image->setQuality( eq::Frame::BUFFER_COLOR, 0.f );
        image->setAlphaUsage( false );

        const GLEWContext* glewContext = glewGetContext();
        std::vector< uint32_t > names;
        image->findTransferers( eq::Frame::BUFFER_COLOR, glewContext, names );

        for( std::vector< uint32_t >::const_iterator j = names.begin();
             j != names.end(); ++j )
        {
            _draw( 0 );

            // setup
            event.formatType[31] = '\0';
            event.data.type = ConfigEvent::READBACK;

            image->allocDownloader( eq::Frame::BUFFER_COLOR, *j, glewContext );
            image->setPixelViewport( pvp );

            const uint32_t outputToken = 
                image->getExternalFormat( eq::Frame::BUFFER_COLOR );
            snprintf( event.formatType, 32, "%s/%x/%x", 
                _enums[i].internalFormatString, outputToken, *j );


            // read
            glFinish();
            size_t nLoops = 0;
            clock.reset();
            while( clock.getTime64() < 100 /*ms*/ )
            {
                image->startReadback( eq::Frame::BUFFER_COLOR, pvp, zoom,
                                      glObjects );
                image->finishReadback( zoom, glObjects->glewGetContext( ));
                ++nLoops;
            }
            glFinish();
            event.msec = clock.getTimef() / float( nLoops );

            const eq::PixelData& pixels =
                image->getPixelData( eq::Frame::BUFFER_COLOR );
            event.area.x() = pixels.pvp.w;             
            event.area.y() = pixels.pvp.h;
            event.dataSizeGPU = pixels.pvp.getArea() * _enums[i].pixelSize;
            event.dataSizeCPU = 
            image->getPixelDataSize( eq::Frame::BUFFER_COLOR );

            GLenum error = glGetError();
            if( error != GL_NO_ERROR )
                event.msec = -static_cast<float>( error );
            config->sendEvent( event );

            eq::Compositor::ImageOp op;
            op.channel = this;
            op.buffers = eq::Frame::BUFFER_COLOR;
            op.offset = offset;
            op.zoom = zoom;

            event.data.type = ConfigEvent::ASSEMBLE;
            event.dataSizeCPU = 
                image->getPixelDataSize( eq::Frame::BUFFER_COLOR );

            clock.reset();
            eq::Compositor::assembleImage( image, op );
            event.msec = clock.getTimef();

            const eq::PixelData& pixelA =
                image->getPixelData( eq::Frame::BUFFER_COLOR );
            event.area.x() = pixelA.pvp.w; 
            event.area.y() = pixelA.pvp.h;
            event.dataSizeGPU =
                image->getPixelDataSize( eq::Frame::BUFFER_COLOR );

            error = glGetError();
            
            if( error != GL_NO_ERROR )
                event.msec = -static_cast<float>( error );
            config->sendEvent( event );
        }
    }
}

void Channel::_testTiledOperations()
{
    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    EQASSERT( images[0] );

    eq::Config* config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const eq::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x() = pvp.w;

    co::base::Clock clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();
    const GLEWContext* glewContext = glewGetContext();

    //----- test tiled assembly algorithms
    eq::PixelViewport subPVP = pvp;
    subPVP.h /= NUM_IMAGES;

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        EQASSERT( images[ i ] );
        images[ i ]->setPixelViewport( subPVP );
    }

    for( unsigned tiles = 0; tiles < NUM_IMAGES; ++tiles )
    {
        _draw( 0 );

        event.area.y() = subPVP.h * (tiles+1);

        //---- readback of 'tiles' depth images
        event.data.type = ConfigEvent::READBACK;
        snprintf( event.formatType, 32, "%d depth tiles", tiles+1 ); 

        event.msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];
            EQCHECK( image->allocDownloader( eq::Frame::BUFFER_DEPTH, 
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                             glewContext ));
            image->clearPixelData( eq::Frame::BUFFER_DEPTH );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_DEPTH, subPVP,
                                  eq::Zoom::NONE, glObjects );
            image->finishReadback( eq::Zoom::NONE, glObjects->glewGetContext( ));
            event.msec += clock.getTimef();
            
        }

        config->sendEvent( event );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage( images[j],
                            "EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT",
                            "tiles" );

        //---- readback of 'tiles' color images
        event.data.type = ConfigEvent::READBACK;
        snprintf( event.formatType, 32, "%d color tiles", tiles+1 );

        event.msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];

            EQCHECK( image->allocDownloader( eq::Frame::BUFFER_COLOR, 
                                            EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
                                              glewContext ));
            image->clearPixelData( eq::Frame::BUFFER_COLOR );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_COLOR, subPVP,
                                  eq::Zoom::NONE, glObjects );
            image->finishReadback( eq::Zoom::NONE, glObjects->glewGetContext( ));
            event.msec += clock.getTimef();
        }
        config->sendEvent( event );

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
        event.data.type = ConfigEvent::ASSEMBLE;
        snprintf( event.formatType, 32, "tiles, GL1.1, %d images", tiles+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= tiles; ++j )
            eq::Compositor::assembleImage( images[j], op );

        event.msec = clock.getTimef();
        config->sendEvent( event );

        // CPU
        snprintf( event.formatType, 32, "tiles, CPU,   %d images", tiles+1 ); 

        std::vector< eq::Frame* > frames;
        frames.push_back( &_frame );

        clock.reset();
        eq::Compositor::assembleFramesCPU( frames, this );
        event.msec = clock.getTimef();
        config->sendEvent( event );
    }
}

void Channel::_testDepthAssemble()
{
    //----- setup constant data
    const eq::Images& images = _frame.getImages();
    eq::Image* image  = images[ 0 ];
    EQASSERT( image );

    eq::Config* config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const eq::Vector2i offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x() = pvp.w;

    co::base::Clock clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();
    const GLEWContext* glewContext = glewGetContext();

    //----- test depth-based assembly algorithms
    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        image = images[ i ];
        EQASSERT( image );
        image->setPixelViewport( pvp );
    }

    event.area.y() = pvp.h;

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        _draw( i );

        // fill depth & color image
        image = images[ i ];

        EQCHECK( image->allocDownloader( eq::Frame::BUFFER_COLOR, 
                                         EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA, 
                                         glewContext ));

        EQCHECK( image->allocDownloader( eq::Frame::BUFFER_DEPTH, 
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                         glewContext ));

        image->clearPixelData( eq::Frame::BUFFER_COLOR );
        image->clearPixelData( eq::Frame::BUFFER_DEPTH );

        image->startReadback( eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH,
                              pvp, eq::Zoom::NONE, glObjects );
        image->finishReadback( eq::Zoom::NONE, glObjects->glewGetContext( ));

        if( i == NUM_IMAGES-1 )
            _saveImage( image,"EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT",
                              "depthAssemble" );

        // benchmark
        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        event.data.type = ConfigEvent::ASSEMBLE;
        snprintf( event.formatType, 32, "depth, GL1.1, %d images", i+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= i; ++j )
            eq::Compositor::assembleImageDB_FF( images[j], op );

        event.msec = clock.getTimef();
        config->sendEvent( event );

        // GLSL
        if( GLEW_VERSION_2_0 )
        {
            snprintf( event.formatType, 32, "depth, GLSL,  %d images", i+1 ); 

            clock.reset();
            for( unsigned j = 0; j <= i; ++j )
                eq::Compositor::assembleImageDB_GLSL( images[j], op );
            event.msec = clock.getTimef();
            config->sendEvent( event );
        }

        // CPU
        snprintf( event.formatType, 32, "depth, CPU,   %d images", i+1 ); 

        std::vector< eq::Frame* > frames;
        frames.push_back( &_frame );

        clock.reset();
        eq::Compositor::assembleFramesCPU( frames, this );
        event.msec = clock.getTimef();
        config->sendEvent( event );
    }
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
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    eq::Channel::frameDraw( spin );

    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );

#if 0
    setNearFar( 0.5f, 5.0f );
    const GLfloat lightPosition[]    = {5.0f, 0.0f, 5.0f, 0.0f};
    const GLfloat lightDiffuse[]     = {0.8f, 0.8f, 0.8f, 1.0f};

    const GLfloat materialDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};

    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );

    glMaterialfv( GL_FRONT, GL_DIFFUSE,   materialDiffuse );

    eq::Matrix4f rotation;
    eq::Vector3f translation;

    translation   = eq::Vector3f::ZERO;
    translation.z = -2.f;
    rotation = eq::Matrix4f::IDENTITY;
    rotation.rotate_x( static_cast<float>( -M_PI_2 ));
    rotation.rotate_y( static_cast<float>( -M_PI_2 ));

    glTranslatef( translation.x, translation.y, translation.z );
    glMultMatrixf( rotation.ml );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glColor3f( 1.f, 1.f, 0.f );
    glNormal3f( 1.f, -1.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
        glVertex3f(  1.f, 10.f,  2.5f );
        glVertex3f( -1.f, 10.f,  2.5f );
        glVertex3f(  1.f,-10.f, -2.5f );
        glVertex3f( -1.f,-10.f, -2.5f );
    glEnd();

#else

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

#endif

    glPopAttrib( );
}


}

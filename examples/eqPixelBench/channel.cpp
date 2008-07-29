
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "config.h"
#include "configEvent.h"

using namespace std;
using namespace eq::base;

#ifdef WIN32_API
#  define snprintf _snprintf
#endif

namespace eqPixelBench
{
namespace
{
struct EnumMap
{
    const char*    formatString;
    const char*    typeString;
    const uint32_t format;
    const uint32_t type;
};

#define ENUM_MAP_ITEM( format, type )          \
    { #format, #type, format, type }

static EnumMap _enums[] = {
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_BYTE ), // initial buffer resize
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_BGRA, GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_RGB,  GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_RGB,  GL_UNSIGNED_INT_8_8_8_8_REV),
    ENUM_MAP_ITEM( GL_BGR,  GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_BGR,  GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_DEPTH_COMPONENT, GL_FLOAT ),
    ENUM_MAP_ITEM( GL_DEPTH_COMPONENT, GL_UNSIGNED_INT ),
    ENUM_MAP_ITEM( GL_DEPTH_STENCIL_NV, GL_UNSIGNED_INT_24_8_NV ),
    { 0, 0, false, false }};

#define NUM_IMAGES 8
}

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
{
    eq::FrameData* frameData = new eq::FrameData;
    _frame.setData( frameData );

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
        frameData->newImage();
}

void Channel::frameStart( const uint32_t frameID, const uint32_t frameNumber ) 
{
    Config*      config = static_cast< Config* >( getConfig( ));
    const Clock* clock  = config->getClock();

    if( clock )
    {
        ConfigEvent   event;
        event.msec = clock->getTimef();

        const string& name  = getName();
        if( name.empty( ))    
            snprintf( event.data.user.data, 32, "%p", this);
        else
            snprintf( event.data.user.data, 32, "%s", name.c_str( ));
        event.data.user.data[31] = '\0';
        event.area.x = 0;
        event.area.y = 0;

        snprintf( event.formatType, 64, "Latency between app and render start");
        event.data.type = ConfigEvent::START_LATENCY;

        config->sendEvent( event );
    }

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameDraw( const uint32_t frameID )
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

    _testFormats();
    _testTiledOperations();
    _testDepthAssemble();
    
    resetAssemblyState();
}

ConfigEvent Channel::_createConfigEvent()
{
    ConfigEvent   event;
    const string& name = getName();

    if( name.empty( ))    
        snprintf( event.data.user.data, 32, "%p", this );
    else
        snprintf( event.data.user.data, 32, "%s", name.c_str( ));

    event.data.user.data[31] = '\0';
    return event;
}

void Channel::_testFormats()
{
    //----- setup constant data
    const eq::ImageVector& images = _frame.getImages();
    eq::Image*             image  = images[ 0 ];
    EQASSERT( image );

    eq::Config*              config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const vmml::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getWindow()->getObjectManager();

    //----- test all default format/type combinations
    glGetError();
    for( uint32_t i=0; _enums[i].formatString; ++i )
    {
        // setup
        snprintf( event.formatType, 64, "%s/%s", 
            _enums[i].formatString, _enums[i].typeString );
        event.formatType[63] = '\0';
        event.data.type = ConfigEvent::READBACK;
        event.area.y = pvp.h;

        image->setFormat( eq::Frame::BUFFER_COLOR, _enums[i].format );
        image->setType(   eq::Frame::BUFFER_COLOR, _enums[i].type );

        // read
        clock.reset();
        image->startReadback( eq::Frame::BUFFER_COLOR, pvp, glObjects );
        image->syncReadback();
        event.msec = clock.getTimef();

        GLenum error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );
        config->sendEvent( event );

        // draw
        event.data.type = ConfigEvent::ASSEMBLE;
        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR;
        op.offset  = offset;

        clock.reset();
        eq::Compositor::assembleImage( image, op );
        event.msec = clock.getTimef();

        error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );

        config->sendEvent( event );

        // read #n
        event.area.y    = pvp.h * NUM_IMAGES;
        event.data.type = ConfigEvent::READBACK;
        snprintf( event.formatType, 64, "%d images, tiled", NUM_IMAGES ); 

        clock.reset();
        for( unsigned j = 0; j < NUM_IMAGES; ++j )
        {
            image = images[ j ];
            image->setFormat( eq::Frame::BUFFER_COLOR, _enums[i].format );
            image->setType(   eq::Frame::BUFFER_COLOR, _enums[i].type );

            image->startReadback( eq::Frame::BUFFER_COLOR, pvp, glObjects );
        }
        for( unsigned j = 0; j < NUM_IMAGES; ++j )
            images[j]->syncReadback();

        event.msec = clock.getTimef();

        error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );

        config->sendEvent( event ); 
        image = images[ 0 ];
    }
}

void Channel::_testTiledOperations()
{
    //----- setup constant data
    const eq::ImageVector& images = _frame.getImages();
    eq::Image*             image  = images[ 0 ];
    EQASSERT( image );

    eq::Config*              config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const vmml::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getWindow()->getObjectManager();

    //----- test tiled assembly algorithms
    eq::PixelViewport subPVP = pvp;
    subPVP.h /= NUM_IMAGES;

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        image = images[ i ];
        EQASSERT( image );
        image->setPixelViewport( subPVP );
    }

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        event.area.y = subPVP.h * (i+1);

        // interleaved readback of 'i' color images
        event.data.type = ConfigEvent::READBACK;
        snprintf( event.formatType, 64, "Readback %d color images", i+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= i; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            image = images[ j ];
            image->setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
            image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );

            image->startReadback( eq::Frame::BUFFER_COLOR, subPVP, glObjects );
        }
        for( unsigned j = 0; j <= i; ++j )
            images[j]->syncReadback();

        event.msec = clock.getTimef();
        config->sendEvent( event );            

        // benchmark assembly operations
        subPVP.y = pvp.y + i * subPVP.h;

        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        event.data.type = ConfigEvent::ASSEMBLE;
        snprintf( event.formatType, 64, 
                  "Tiled assembly (GL1.1) of %d images", i+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= i; ++j )
            eq::Compositor::assembleImage( images[j], op );

        event.msec = clock.getTimef();
        config->sendEvent( event );            

        // CPU
        snprintf( event.formatType, 64,
                  "Tiled assembly (CPU)   of %d images", i+1 ); 

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
    const eq::ImageVector& images = _frame.getImages();
    eq::Image*             image  = images[ 0 ];
    EQASSERT( image );

    eq::Config*              config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const vmml::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getWindow()->getObjectManager();


    //----- test depth-based assembly algorithms
    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        image = images[ i ];
        EQASSERT( image );
        image->setPixelViewport( pvp );
    }

    event.area.y = pvp.h;

    for( unsigned i = 0; i < NUM_IMAGES; ++i )
    {
        // fill depth & color image
        image = images[ i ];
        image->setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
        image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
        image->setFormat( eq::Frame::BUFFER_DEPTH, GL_DEPTH_COMPONENT );
        image->setType(   eq::Frame::BUFFER_DEPTH, GL_FLOAT );

        image->startReadback( eq::Frame::BUFFER_COLOR | 
                              eq::Frame::BUFFER_DEPTH, pvp, glObjects );
        image->syncReadback();

        // benchmark
        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        event.data.type = ConfigEvent::ASSEMBLE;
        snprintf( event.formatType, 64, 
                  "Depth-based assembly (GL1.1) of %d images", i+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= i; ++j )
            eq::Compositor::assembleImageDB_FF( images[j], op );

        event.msec = clock.getTimef();
        config->sendEvent( event );            

        // GLSL
        if( GLEW_VERSION_2_0 )
        {
            snprintf( event.formatType, 64,
                      "Depth-based assembly (GLSL)  of %d images", i+1 ); 

            clock.reset();
            for( unsigned j = 0; j <= i; ++j )
                eq::Compositor::assembleImageDB_GLSL( images[j], op );
            event.msec = clock.getTimef();
            config->sendEvent( event );
        }

        // CPU
        snprintf( event.formatType, 64, 
                  "Depth-based assembly (CPU)   of %d images", i+1 ); 

        std::vector< eq::Frame* > frames;
        frames.push_back( &_frame );

        clock.reset();
        eq::Compositor::assembleFramesCPU( frames, this );
        event.msec = clock.getTimef();
        config->sendEvent( event );            
    }
}

}

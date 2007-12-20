
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "configEvent.h"

using namespace std;
using namespace eqBase;

#ifdef WIN32_API
#  define snprintf _snprintf
#endif

namespace eqPixelBench
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

static EnumMap enums[] = {
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
    ENUM_MAP_ITEM( GL_DEPTH_STENCIL_NV, GL_UNSIGNED_INT_24_8_NV ),
    { 0, 0, false, false }};

void Channel::frameDraw( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    setupAssemblyState();
    
    Clock                    clock;
    eq::Image                image;
    const eq::PixelViewport& pvp    = getPixelViewport();
    const vmml::Vector2i     offset( pvp.x, pvp.y );
    eq::Config*              config = getConfig();

    ConfigEvent   event;
    const string& name  = getName();
    if( name.empty( ))    
        snprintf( event.data.user.data, 32, "%p", this);
    else
        snprintf( event.data.user.data, 32, "%s", name.c_str( ));
    event.data.user.data[31] = '\0';
    event.area.x = pvp.w;
    event.area.y = pvp.h;

    glGetError();
    for( uint32_t i=0; enums[i].formatString; ++i )
    {
        // setup
        snprintf( event.formatType, 64, "%s/%s", 
                  enums[i].formatString, enums[i].typeString );
        event.formatType[63] = '\0';
        event.data.type = ConfigEvent::READBACK;

        image.setFormat( eq::Frame::BUFFER_COLOR, enums[i].format );
        image.setType(   eq::Frame::BUFFER_COLOR, enums[i].type );
        
        // read
        clock.reset();
        image.startReadback( eq::Frame::BUFFER_COLOR, pvp );
        image.syncReadback();
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
        eq::Compositor::assembleImage( &image, op );
        event.msec = clock.getTimef();

        error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );

        config->sendEvent( event );

        if( enums[i].format == GL_DEPTH_COMPONENT && enums[i].type == GL_FLOAT )
        {
            // test z-based assembly
            // fill depth & color image
            image.setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
            image.setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
            image.setFormat( eq::Frame::BUFFER_DEPTH, enums[i].format );
            image.setType(   eq::Frame::BUFFER_DEPTH, enums[i].type );
        
            image.startReadback( eq::Frame::BUFFER_COLOR | 
                                 eq::Frame::BUFFER_DEPTH, pvp );
            image.syncReadback();

            // benchmark
            op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;

            snprintf( event.formatType, 64, 
                      "Fixed-function depth-based assembly" ); 
            clock.reset();
            eq::Compositor::assembleImageDB_FF( &image, op );
            event.msec = clock.getTimef();
            config->sendEvent( event );            

            if( GLEW_VERSION_2_0 )
            {
                snprintf( event.formatType, 64, "GLSL depth-based assembly" ); 
                clock.reset();
                eq::Compositor::assembleImageDB_GLSL( &image, op );
                event.msec = clock.getTimef();
                config->sendEvent( event );
            }
        }
    }

    resetAssemblyState();
}
}

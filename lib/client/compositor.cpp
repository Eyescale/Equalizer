
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compositor.h"

#include "channel.h"
#include "frame.h"
#include "log.h"
#include "image.h"
#include "statEvent.h"
#include "windowSystem.h"

#include <eq/base/monitor.h>

using eqBase::Monitor;
using namespace std;

namespace eq
{

#define glewGetContext op.channel->glewGetContext

void Compositor::assembleFrames( const vector<Frame*>& frames, Channel* channel)
{
    // This is an optimized assembly version. The frames are not assembled in
    // the saved order, but in the order they become available, which is faster
    // because less time is spent waiting on frame availability.
    //
    // The ready frames are counted in a monitor. Whenever a frame becomes
    // available, it increments the monitor which causes this code to wake up
    // and assemble it.

    Monitor<uint32_t>     monitor;

    // register monitor with all input frames
    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->addListener( monitor );
    }

    uint32_t       nUsedFrames  = 0;
    vector<Frame*> unusedFrames = frames;

    // wait and assemble frames
    while( !unusedFrames.empty( ))
    {
        {
            StatEvent event( StatEvent::CHANNEL_WAIT_FRAME, channel );
            monitor.waitGE( ++nUsedFrames );
        }

        for( vector<Frame*>::iterator i = unusedFrames.begin();
             i != unusedFrames.end(); ++i )
        {
            Frame* frame = *i;
            if( !frame->isReady( ))
                continue;

            assembleFrame( frame, channel );
            unusedFrames.erase( i );
            break;
        }
    }

    // de-register the monitor
    for( vector<Frame*>::const_iterator i = frames.begin(); i != frames.end();
         ++i )
    {
        Frame* frame = *i;
        // syncAssembleFrame( frame );
        frame->removeListener( monitor );
    }
}

void Compositor::assembleFramesSorted( const vector<Frame*>& frames,
                                       Channel* channel )
{
    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->waitReady( );
        assembleFrame( frame, channel );
    }
}

void Compositor::assembleFrame( const Frame* frame, Channel* channel )
{
    const vector< Image* >& images = frame->getImages();
    if( images.empty( ))
        EQWARN << "No images to assemble" << endl;

    ImageOp operation;
    operation.channel = channel;
    operation.buffers = frame->getBuffers();
    operation.offset  = frame->getOffset();
    operation.pixel   = frame->getPixel();

    for( vector<Image*>::const_iterator i = images.begin(); 
         i != images.end(); ++i )
    {
        const Image* image = *i;
        assembleImage( image, operation );
    }
}

void Compositor::assembleImage( const Image* image, const ImageOp& op )
{
    ImageOp operation = op;
    operation.buffers = Frame::BUFFER_NONE;

    if( op.buffers & Frame::BUFFER_COLOR && 
        image->hasPixelData( Frame::BUFFER_COLOR ))

        operation.buffers |= Frame::BUFFER_COLOR;

    if( op.buffers & Frame::BUFFER_DEPTH &&
        image->hasPixelData( Frame::BUFFER_DEPTH ))

        operation.buffers |= Frame::BUFFER_DEPTH;

    if( operation.buffers == Frame::BUFFER_NONE )
    {
        EQWARN << "No image attachement buffers to assemble" << endl;
        return;
    }

    setupStencilBuffer( image, operation );

    if( operation.buffers == Frame::BUFFER_COLOR )
        assembleImage2D( image, operation );
    else if( operation.buffers == ( Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
        assembleImageDB( image, operation );
    else
        EQWARN << "Don't know how to assemble using buffers " 
               << operation.buffers << endl;
}

void Compositor::setupStencilBuffer( const Image* image, const ImageOp& op )
{
    if( op.pixel == Pixel::ALL )
        return;

    glPixelZoom( static_cast< float >( op.pixel.size ), 1.0f );

    // mark stencil buffer where pixel shall pass
    // TODO: OPT!
    glClear( GL_STENCIL_BUFFER_BIT );
    glEnable( GL_STENCIL_TEST );
    glEnable( GL_DEPTH_TEST );

    glStencilFunc( GL_ALWAYS, 1, 1 );
    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

    glLineWidth( 1.0f );
    glDepthMask( false );
    glColorMask( false, false, false, false );
    
    const PixelViewport& pvp    = image->getPixelViewport();
    const int32_t        startX = op.offset.x + pvp.x;
    const int32_t        endX   = startX   + pvp.w * op.pixel.size;
    const float          startY = static_cast< float >( op.offset.y + pvp.y );
    const float          endY   = static_cast< float >( startY   + pvp.h );

    glBegin( GL_LINES );
    for( int32_t x = startX + op.pixel.index; x < endX; x += op.pixel.size )
    {
        glVertex3f( static_cast< float >( x ), startY, 0.0f );
        glVertex3f( static_cast< float >( x ), endY, 0.0f );        
    }
    glEnd();
    
    glDisable( GL_DEPTH_TEST );
    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    
    op.channel->applyColorMask();
    glDepthMask( true );
}

void Compositor::assembleImage2D( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImage2D " << pvp << endl;
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

    glRasterPos2i( op.offset.x + pvp.x, op.offset.y + pvp.y );
    glDrawPixels( pvp.w, pvp.h, 
                  image->getFormat( Frame::BUFFER_COLOR ), 
                  image->getType( Frame::BUFFER_COLOR ), 
                  image->getPixelData( Frame::BUFFER_COLOR ));
}

void Compositor::assembleImageDB( const Image* image, const ImageOp& op )
{
    if( GLEW_VERSION_2_0 )
        assembleImageDB_GLSL( image, op );
    else
        assembleImageDB_FF( image, op );
}

void Compositor::assembleImageDB_FF( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImageDB, fixed function " << pvp 
                          << endl;
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
    EQASSERT( image->hasPixelData( Frame::BUFFER_DEPTH ));

    // Z-Based sort-last assembly
    glRasterPos2i( op.offset.x + pvp.x, op.offset.y + pvp.y );
    glEnable( GL_STENCIL_TEST );
    
    // test who is in front and mark in stencil buffer
    glEnable( GL_DEPTH_TEST );

    const bool pixelComposite = ( op.pixel != Pixel::ALL );
    if( pixelComposite )
    {   // keep already marked stencil values
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_ZERO, GL_REPLACE );
    }
    else
    {
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );
    }

    glDrawPixels( pvp.w, pvp.h, image->getFormat( Frame::BUFFER_DEPTH ), 
                  image->getType( Frame::BUFFER_DEPTH ), 
                  image->getPixelData( Frame::BUFFER_DEPTH ));
    
    glDisable( GL_DEPTH_TEST );

    // draw front-most, visible pixels using stencil mask
    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_ZERO, GL_ZERO );
    
    glDrawPixels( pvp.w, pvp.h, image->getFormat( Frame::BUFFER_COLOR ), 
                  image->getType( Frame::BUFFER_COLOR ),
                  image->getPixelData( Frame::BUFFER_COLOR ));

    glDisable( GL_STENCIL_TEST );
}

namespace
{
static const char  _assembleDBKeys[3] = "12";
static const void* KEY_DB_DEPTH_TEXTURE = &_assembleDBKeys[0];
static const void* KEY_DB_COLOR_TEXTURE = &_assembleDBKeys[1];
static const void* KEY_DB_GLSL          = &_assembleDBKeys[2];
}

void Compositor::assembleImageDB_GLSL( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImageDB, GLSL " << pvp 
                          << endl;

    Window*                window  = op.channel->getWindow();
    Window::ObjectManager* objects = window->getObjectManager();
    GLuint depthTexture = objects->obtainTexture( KEY_DB_DEPTH_TEXTURE );
    GLuint colorTexture = objects->obtainTexture( KEY_DB_COLOR_TEXTURE );
    GLuint program      = objects->getProgram( KEY_DB_GLSL );

    if( program == Window::ObjectManager::FAILED )
    {
        // Create fragment shader which reads color and depth values from 
        // rectangular textures
        const GLuint shader = objects->newShader( KEY_DB_GLSL,
                                                  GL_FRAGMENT_SHADER );
        EQASSERT( shader != Window::ObjectManager::FAILED );

        const char* source = "uniform sampler2DRect color; uniform sampler2DRect depth; void main(void){ gl_FragColor = texture2DRect( color, gl_TexCoord[0].st ); gl_FragDepth = texture2DRect( depth, gl_TexCoord[0].st ).x; }";

        EQ_GL_CALL( glShaderSource( shader, 1, &source, 0 ));
        EQ_GL_CALL( glCompileShader( shader ));

        GLint status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
        if( !status )
            EQERROR << "Failed to compile fragment shader for DB compositing" 
                    << endl;

        program = objects->newProgram( KEY_DB_GLSL );

        EQ_GL_CALL( glAttachShader( program, shader ));
        EQ_GL_CALL( glLinkProgram( program ));

        glGetProgramiv( program, GL_LINK_STATUS, &status );
        if( !status )
        {
            EQWARN << "Failed to link shader program for DB compositing" 
                   << endl;
            return;
        }

        // use fragment shader and setup uniforms
        EQ_GL_CALL( glUseProgram( program ));
        
        const GLint depthParam = glGetUniformLocation( program, "depth" );
        glUniform1i( depthParam, 0 );
        const GLint colorParam = glGetUniformLocation( program, "color" );
        glUniform1i( colorParam, 1 );
    }
    else
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));

    // Enable & download color and depth textures
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE0 ));
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_RECTANGLE_ARB, depthTexture ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    EQ_GL_CALL( glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                              GL_DEPTH_COMPONENT32_ARB,
                              pvp.w, pvp.h, 0,
                              image->getFormat( Frame::BUFFER_DEPTH ), 
                              image->getType( Frame::BUFFER_DEPTH ),
                              image->getPixelData( Frame::BUFFER_DEPTH )));

    EQ_GL_CALL( glActiveTexture( GL_TEXTURE1 ));
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_RECTANGLE_ARB, colorTexture ));
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_NEAREST );

    EQ_GL_CALL( glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                              GL_RGBA,
                              pvp.w, pvp.h, 0,
                              image->getFormat( Frame::BUFFER_COLOR ), 
                              image->getType( Frame::BUFFER_COLOR ),
                              image->getPixelData( Frame::BUFFER_COLOR )));

    // Draw a quad using shader & textures in the right place
    glEnable( GL_DEPTH_TEST );
    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = static_cast< float >
        ( op.offset.x + pvp.x * op.pixel.size + op.pixel.index );
    const float endX   = static_cast< float >
        ( op.offset.x + (pvp.x + pvp.w) * op.pixel.size + op.pixel.index );

    const float startY = static_cast< float >( op.offset.y + pvp.y );
    const float endY   = static_cast< float >( op.offset.y + pvp.y + pvp.h );

    glBegin( GL_TRIANGLE_STRIP );
    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, 0.0f );
    glVertex3f( startX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, pvp.w, 0.0f );
    glMultiTexCoord2f( GL_TEXTURE1, pvp.w, 0.0f );
    glVertex3f( endX, startY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, 0.0f, pvp.h );
    glMultiTexCoord2f( GL_TEXTURE1, 0.0f, pvp.h );
    glVertex3f( startX, endY, 0.0f );

    glMultiTexCoord2f( GL_TEXTURE0, pvp.w, pvp.h );
    glMultiTexCoord2f( GL_TEXTURE1, pvp.w, pvp.h );
    glVertex3f( endX, endY, 0.0f );

    glEnd();

#ifdef Darwin
    // WAR for bug 1849962. If we don't flush the GL pipe here, subsequent
    // assemble ops before the next swap get random texture values form either
    // this op or the next.
    glFlush();
#endif
            
    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_DEPTH_TEST );
    EQ_GL_CALL( glUseProgram( 0 ));
}


}

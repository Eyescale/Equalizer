
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include <eq/base/perThread.h>

#include "compositor.h"

#include "channel.h"
#include "frame.h"
#include "log.h"
#include "image.h"
#include "statEvent.h"
#include "windowSystem.h"

#include <eq/base/executionListener.h>
#include <eq/base/monitor.h>

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

using eqBase::Monitor;
using namespace std;

namespace eq
{

#define glewGetContext op.channel->glewGetContext

namespace
{
// use to address one shader and program per shared context set
static const char glslKey = 42;
// Image used for CPU-based assembly
static eqBase::PerThread< Image* > _resultImage;

// Deletes the per-thread result image when a thread exits.
class ImageDestructor : public eqBase::ExecutionListener
{
public:
    virtual ~ImageDestructor() {}
    virtual void notifyExecutionStopping()
        {
            Image* image = _resultImage.get();
            _resultImage = 0;
            delete image;
        }
};

static  ImageDestructor _imageDestructor;


static bool _useCPUAssembly( const vector< Frame* >& frames )
{
    // It doesn't make sense to use CPU-assembly for only one frame
    if( frames.size() < 2 )
        return false;

    // Test that at least two input frames have color and depth buffers. We
    // assume then that we will have at least one color&depth image per frame so
    // most likely it's worth to wait for the images and to do a CPU-based
    // assembly.
    size_t nFrames = 0;
    for( vector< Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        if( frame->getPixel() != Pixel::ALL ) // Not supported yet on the CPU
            return false;

        if( frame->getBuffers() == (Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH ))
            ++nFrames;
    }

    if( nFrames < 2 )
        return false;

    // Now wait for all images to be ready and test if our assumption was
    // correct, that there are enough images to make a CPU-based assembly
    // worthwhile and all other preconditions for our current CPU-based assembly
    // code are true.
    size_t nImages = 0;

    for( vector< Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame* frame = *i;
        frame->waitReady();

        const vector< Image* >& images = frame->getImages();        
        for( vector< Image* >::const_iterator j = images.begin(); 
             j != images.end(); ++j )
        {
            const Image* image = *j;

            if( image->hasPixelData( Frame::BUFFER_COLOR ) &&
                image->hasPixelData( Frame::BUFFER_DEPTH ))

                ++nImages;
        }
    }

    return (nImages > 1);
}
}

void Compositor::assembleFrames( const vector< Frame* >& frames,
                                 Channel* channel )
{
    if( _useCPUAssembly( frames ))
        assembleFramesCPU( frames, channel );
    else
        assembleFramesUnsorted( frames, channel );
}

void Compositor::assembleFramesSorted( const vector< Frame* >& frames,
                                       Channel* channel )
{
    if( _useCPUAssembly( frames ))
    {
        assembleFramesCPU( frames, channel );
        return;
    }

    for( vector< Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->waitReady( );
        assembleFrame( frame, channel );
    }
}

void Compositor::assembleFramesUnsorted( const vector< Frame* >& frames, 
                                         Channel* channel )
{
    EQVERB << "Unsorted GPU assembly" << endl;
    // This is an optimized assembly version. The frames are not assembled in
    // the saved order, but in the order they become available, which is faster
    // because less time is spent waiting on frame availability.
    //
    // The ready frames are counted in a monitor. Whenever a frame becomes
    // available, it increments the monitor which causes this code to wake up
    // and assemble it.

    // register monitor with all input frames
    Monitor<uint32_t> monitor;
    for( vector< Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->addListener( monitor );
    }

    uint32_t       nUsedFrames  = 0;
    vector< Frame* > unusedFrames = frames;

    // wait and assemble frames
    while( !unusedFrames.empty( ))
    {
        {
            ScopedStatistics event( StatEvent::CHANNEL_WAIT_FRAME, channel );
            monitor.waitGE( ++nUsedFrames );
        }

        for( vector< Frame* >::iterator i = unusedFrames.begin();
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
    for( vector< Frame* >::const_iterator i = frames.begin(); i != frames.end();
         ++i )
    {
        Frame* frame = *i;
        // syncAssembleFrame( frame );
        frame->removeListener( monitor );
    }
}

void Compositor::assembleFramesCPU( const vector< Frame* >& frames,
                                    Channel* channel )
{
    // Assembles images from DB and 2D compounds using the CPU and then
    // assembles the result image. Does not yet support Pixel or Eye
    // compounds. The result image has to be fully filled.

    const Image* result = assembleFramesCPU( frames );
    if( !result )
        return;

    // assemble result on dest channel
    ImageOp operation;
    operation.channel = channel;
    operation.buffers = Frame::BUFFER_COLOR | Frame::BUFFER_DEPTH;
    assembleImage( result, operation );

#if 0
    static uint32_t counter = 0;
    ostringstream stringstream;
    stringstream << "Image_" << ++counter;
    result->writeImages( stringstream.str( ));
#endif
}

const Image* Compositor::assembleFramesCPU( const std::vector< Frame* >& frames)
{
    EQVERB << "Sorted CPU assembly" << endl;

    // collect and categorize input images
    vector< FrameImage > imagesDB;
    vector< FrameImage > images2D;
    PixelViewport        resultPVP;

    int colorFormat = GL_BGRA;
    int colorType   = GL_UNSIGNED_BYTE;
    int depthFormat = GL_DEPTH_COMPONENT;
    int depthType   = GL_FLOAT;

    for( vector< Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->waitReady();

        EQASSERTINFO( frame->getPixel() == Pixel::ALL,
                      "CPU-based pixel recomposition not implemented" );

        const vector< Image* >& images = frame->getImages();        
        for( vector< Image* >::const_iterator j = images.begin(); 
             j != images.end(); ++j )
        {
            const Image* image = *j;
            EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

            resultPVP.merge( image->getPixelViewport() + frame->getOffset( ));
            
            colorFormat = image->getFormat( Frame::BUFFER_COLOR );
            colorType   = image->getType( Frame::BUFFER_COLOR );

            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                imagesDB.push_back( FrameImage( frame, image ));
                depthFormat = image->getFormat( Frame::BUFFER_DEPTH );
                depthType   = image->getType( Frame::BUFFER_DEPTH );
            }
            else
                images2D.push_back( FrameImage( frame, image ));
        }
    }

    if( !resultPVP.hasArea( ))
    {
        EQWARN << "Nothing to assemble" << endl;
        return 0;
    }

    // prepare output image
    Image* result = _resultImage.get();
    if( !result )
    {
        eqBase::Thread::addListener( &_imageDestructor );

        result       = new Image;
        _resultImage = result;
    }

    result->setFormat( Frame::BUFFER_COLOR, colorFormat );
    result->setType(   Frame::BUFFER_COLOR, colorType );
    result->setFormat( Frame::BUFFER_DEPTH, depthFormat );
    result->setType(   Frame::BUFFER_DEPTH, depthType );
    result->setPixelViewport( resultPVP );
    result->clearPixelData( Frame::BUFFER_COLOR );
    if( !imagesDB.empty( ))
        result->clearPixelData( Frame::BUFFER_DEPTH );

    // assembly
    _assembleDBImages( result, imagesDB );
    _assemble2DImages( result, images2D );

    return result;
}


void Compositor::_assembleDBImages( Image* result, 
                                    const vector< FrameImage >& images )
{
    if( images.empty( ))
        return;

    EQASSERT( result->getFormat( Frame::BUFFER_DEPTH ) == GL_DEPTH_COMPONENT);
    EQASSERT( result->getType( Frame::BUFFER_DEPTH )   == GL_FLOAT );

    const eq::PixelViewport resultPVP = result->getPixelViewport();
    uint32_t* destColor = reinterpret_cast< uint32_t* >
        ( result->getPixelData( Frame::BUFFER_COLOR ));
    float*    destDepth = reinterpret_cast< float* >
        ( result->getPixelData( Frame::BUFFER_DEPTH ));

    for( vector< FrameImage >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const Frame*          frame  = i->first;
        const Image*          image  = i->second;
        const vmml::Vector2i& offset = frame->getOffset();
        const PixelViewport&  pvp    = image->getPixelViewport();
        const int32_t         destX  = offset.x + pvp.x - resultPVP.x;
        const int32_t         destY  = offset.y + pvp.y - resultPVP.y;

        EQASSERT( image->getDepth( Frame::BUFFER_COLOR ) == 4 );
        EQASSERT( image->getDepth( Frame::BUFFER_DEPTH ) == 4 );
        EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
        EQASSERT( image->hasPixelData( Frame::BUFFER_DEPTH ));
        EQASSERTINFO( image->getFormat( Frame::BUFFER_COLOR ) ==
                      result->getFormat( Frame::BUFFER_COLOR ),
                      image->getFormat( Frame::BUFFER_COLOR ) << " != "
                      << result->getFormat( Frame::BUFFER_COLOR ));
        EQASSERT( image->getType( Frame::BUFFER_COLOR ) ==
                  result->getType( Frame::BUFFER_COLOR ));
        EQASSERT( image->getFormat( Frame::BUFFER_DEPTH ) ==
                  result->getFormat( Frame::BUFFER_DEPTH ));
        EQASSERT( image->getType( Frame::BUFFER_DEPTH ) ==
                  result->getType( Frame::BUFFER_DEPTH ));

        const uint32_t* color = reinterpret_cast< const uint32_t* >
            ( image->getPixelData( Frame::BUFFER_COLOR ));
        const float*   depth = reinterpret_cast< const float* >
            ( image->getPixelData( Frame::BUFFER_DEPTH ));

// This crashes in OpenMP when compiled with g++-4.2 !?
//#  pragma omp parallel for
        for( int32_t y = 0; y < pvp.h; ++y )
        {
            const uint32_t skip =  (destY + y) * resultPVP.w + destX;
            EQASSERT( skip * sizeof( int32_t ) <= 
                      result->getPixelDataSize( Frame::BUFFER_COLOR ));
            EQASSERT( skip * sizeof( float ) <= 
                      result->getPixelDataSize( Frame::BUFFER_DEPTH ));

            uint32_t*   destColorIt = destColor + skip;
            float*      destDepthIt = destDepth + skip;
            const uint32_t* colorIt = color + y * pvp.w;
            const float*    depthIt = depth + y * pvp.w;

            for( int32_t x = 0; x < pvp.w; ++x )
            {
                if( *destDepthIt > *depthIt )
                {
                    *destColorIt = *colorIt;
                    *destDepthIt = *depthIt;
                }

                ++destColorIt;
                ++destDepthIt;
                ++colorIt;
                ++depthIt;
            }
        }
    }
}

void Compositor::_assemble2DImages( Image* result, 
                                    const vector< FrameImage >& images )
{
    // This is in large part copy&paste code from _assembleDBImages :-/
    
    const eq::PixelViewport resultPVP = result->getPixelViewport();
    uint8_t* destColor = result->getPixelData( Frame::BUFFER_COLOR );
    uint8_t* destDepth = result->hasPixelData( Frame::BUFFER_DEPTH ) ?
        reinterpret_cast< uint8_t* >(result->getPixelData( Frame::BUFFER_DEPTH))
        : 0;

    for( vector< FrameImage >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const Frame*          frame  = i->first;
        const Image*          image  = i->second;
        const vmml::Vector2i& offset = frame->getOffset();
        const PixelViewport&  pvp    = image->getPixelViewport();
        const int32_t         destX  = offset.x + pvp.x - resultPVP.x;
        const int32_t         destY  = offset.y + pvp.y - resultPVP.y;

        EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));
        EQASSERT( image->getFormat( Frame::BUFFER_COLOR ) ==
                  result->getFormat( Frame::BUFFER_COLOR ));
        EQASSERT( image->getType( Frame::BUFFER_COLOR ) ==
                  result->getType( Frame::BUFFER_COLOR ));

        const uint8_t*   color = image->getPixelData( Frame::BUFFER_COLOR );
        const size_t pixelSize = image->getDepth( Frame::BUFFER_COLOR );
        const size_t rowLength = pvp.w * pixelSize;

// This crashes in OpenMP when compiled with g++-4.2 !?
//#  pragma omp parallel for
        for( int32_t y = 0; y < pvp.h; ++y )
        {
            const size_t skip = ( (destY + y) * resultPVP.w + destX ) *
                                    pixelSize;
            EQASSERT( skip + rowLength <= 
                      result->getPixelDataSize( Frame::BUFFER_COLOR ));
            
            memcpy( destColor + skip, color + y * pvp.w * pixelSize, rowLength);
            if( destDepth )
            {
                EQASSERT( pixelSize == image->getDepth( Frame::BUFFER_DEPTH ));
                bzero( destDepth + skip, rowLength );
            }
        }
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

    for( vector< Image* >::const_iterator i = images.begin(); 
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
        EQWARN << "No image attachment buffers to assemble" << endl;
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

#ifdef EQ_PIXEL_Y
    glPixelZoom( 1.0f, static_cast< float >( op.pixel.size ));

    const float startX = static_cast< float >( op.offset.x + pvp.x );
    const float endX   = static_cast< float >( startX      + pvp.w );
    const float height = static_cast< float >( pvp.h * op.pixel.size );
    const float startY = static_cast< float >( op.offset.y + pvp.y ) + 0.5f;
    const float endY   = startY + height

    glBegin( GL_LINES );
    for( float y = startY + op.pixel.index; y < endY;
         y += static_cast< float >( op.pixel.size ))
    {
        glVertex3f( startX, y, 0.0f );
        glVertex3f( endX,   y, 0.0f );        
    }
    glEnd();
#else
    glPixelZoom( static_cast< float >( op.pixel.size ), 1.0f );

    const float width  = static_cast< float >( pvp.w * op.pixel.size );
    const float startX = static_cast< float >( op.offset.x + pvp.x ) + 0.5f;
    const float endX   = startX + width;
    const float startY = static_cast< float >( op.offset.y + pvp.y );
    const float endY   = static_cast< float >( startY      + pvp.h );

    glBegin( GL_LINES );
    for( float x = startX + op.pixel.index; x < endX; 
         x += static_cast< float >( op.pixel.size ))
    {
        glVertex3f( x, startY, 0.0f );
        glVertex3f( x, endY, 0.0f );        
    }
    glEnd();
#endif
    
    glDisable( GL_DEPTH_TEST );
    glStencilFunc( GL_EQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    
    op.channel->applyColorMask();
    glDepthMask( true );
}

namespace
{
static void _drawRect( const PixelViewport& rect )
{
#ifndef NDEBUG
    if( !getenv( "EQ_TAINT_CHANNELS" ))
        return;

    glDisable( GL_LIGHTING );
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINE_LOOP );
    {
        glVertex3f( rect.x, rect.y, 0.f );
        glVertex3f( rect.getXEnd(), rect.y, 0.f );
        glVertex3f( rect.getXEnd(), rect.getYEnd(), 0.f );
        glVertex3f( rect.x, rect.getYEnd(), 0.f );
    } 
    glEnd();
    glEnable( GL_LIGHTING );
#endif
}
}

void Compositor::assembleImage2D( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImage2D " << pvp << " offset " 
                          << op.offset << endl;
    EQASSERT( image->hasPixelData( Frame::BUFFER_COLOR ));

    glRasterPos2i( op.offset.x + pvp.x, op.offset.y + pvp.y );
    glDrawPixels( pvp.w, pvp.h, 
                  image->getFormat( Frame::BUFFER_COLOR ), 
                  image->getType( Frame::BUFFER_COLOR ), 
                  image->getPixelData( Frame::BUFFER_COLOR ));
    _drawRect( (pvp + op.offset) * op.pixel );
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

void Compositor::assembleImageDB_GLSL( const Image* image, const ImageOp& op )
{
    const PixelViewport& pvp = image->getPixelViewport();

    EQLOG( LOG_ASSEMBLY ) << "assembleImageDB, GLSL " << pvp 
                          << endl;

    Window*                window  = op.channel->getWindow();
    Window::ObjectManager* objects = window->getObjectManager();
    
    // use per-window textures: using a shared texture across multiple windows
    // creates undefined texture contents, since each context has it's own
    // command buffer which is flushed unpredictably (at least on Darwin). A
    // glFlush at the end of this method would do it too, but the performance
    // use case is one window per pipe, hence we'll optimize for that and remove
    // the glFlush.
    const char*            key     = reinterpret_cast< char* >( window );

    GLuint depthTexture = objects->obtainTexture( key );
    GLuint colorTexture = objects->obtainTexture( key+1 );
    GLuint program      = objects->getProgram( &glslKey );

    if( program == Window::ObjectManager::FAILED )
    {
        // Create fragment shader which reads color and depth values from 
        // rectangular textures
        const GLuint shader = objects->newShader( &glslKey, GL_FRAGMENT_SHADER);
        EQASSERT( shader != Window::ObjectManager::FAILED );

        const char* source = "uniform sampler2DRect color; uniform sampler2DRect depth; void main(void){ gl_FragColor = texture2DRect( color, gl_TexCoord[0].st ); gl_FragDepth = texture2DRect( depth, gl_TexCoord[0].st ).x; }";

        EQ_GL_CALL( glShaderSource( shader, 1, &source, 0 ));
        EQ_GL_CALL( glCompileShader( shader ));

        GLint status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
        if( !status )
            EQERROR << "Failed to compile fragment shader for DB compositing" 
                    << endl;

        program = objects->newProgram( &glslKey );

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

        // make sure the other shared context see the program
        glFlush();
    }
    else
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));

    // Enable & download color and depth textures
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

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

    // Draw a quad using shader & textures in the right place
    glEnable( GL_DEPTH_TEST );
    glColor3f( 1.0f, 1.0f, 1.0f );

#ifdef EQ_PIXEL_Y
    const float startX = static_cast< float >( op.offset.x + pvp.x );
    const float endX   = static_cast< float >( op.offset.x + pvp.x + pvp.w );

    const float startY = static_cast< float >
        ( op.offset.y + pvp.y * op.pixel.size + op.pixel.index );
    const float endY   = static_cast< float >
        ( op.offset.y + (pvp.y + pvp.h) * op.pixel.size + op.pixel.index );
#else
    const float startX = static_cast< float >
        ( op.offset.x + pvp.x * op.pixel.size + op.pixel.index );
    const float endX   = static_cast< float >
        ( op.offset.x + (pvp.x + pvp.w) * op.pixel.size + op.pixel.index );

    const float startY = static_cast< float >( op.offset.y + pvp.y );
    const float endY   = static_cast< float >( op.offset.y + pvp.y + pvp.h );
#endif

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

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_DEPTH_TEST );
    EQ_GL_CALL( glUseProgram( 0 ));
}


}

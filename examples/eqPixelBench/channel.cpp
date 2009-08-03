
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#pragma warning(disable: 411) // class defines no constructor to initialize ...
struct EnumMap
{
    const char*    formatString;
    const char*    typeString;
    const uint32_t format;
    const uint32_t type;
};
#pragma warning(default: 411)

#define ENUM_MAP_ITEM( format, type )          \
    { #format, #type, format, type }

static EnumMap _enums[] = {
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_BYTE ), // initial buffer resize
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_RGBA, GL_HALF_FLOAT ),
    ENUM_MAP_ITEM( GL_RGBA, GL_FLOAT ),
    ENUM_MAP_ITEM( GL_BGRA, GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_BGRA, GL_HALF_FLOAT ),
    ENUM_MAP_ITEM( GL_BGRA, GL_FLOAT ),
    ENUM_MAP_ITEM( GL_RGB,  GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_RGB,  GL_UNSIGNED_INT_8_8_8_8_REV),
    ENUM_MAP_ITEM( GL_RGB,  GL_HALF_FLOAT ),
    ENUM_MAP_ITEM( GL_RGB,  GL_FLOAT ),
    ENUM_MAP_ITEM( GL_BGR,  GL_UNSIGNED_BYTE ),
    ENUM_MAP_ITEM( GL_BGR,  GL_UNSIGNED_INT_8_8_8_8_REV ),
    ENUM_MAP_ITEM( GL_BGR,  GL_HALF_FLOAT ),
    ENUM_MAP_ITEM( GL_BGR,  GL_FLOAT ),
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
        event.area.x() = 0;
        event.area.y() = 0;

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
    const eq::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x() = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();

    //----- test all default format/type combinations
    glGetError();
    for( uint32_t i=0; _enums[i].formatString; ++i )
    {
        _draw( 0 );

        // setup
        snprintf( event.formatType, 64, "%s/%s", 
            _enums[i].formatString, _enums[i].typeString );
        event.formatType[63] = '\0';
        event.data.type = ConfigEvent::READBACK;
        event.area.y() = pvp.h;

        image->setFormat( eq::Frame::BUFFER_COLOR, _enums[i].format );
        image->setType(   eq::Frame::BUFFER_COLOR, _enums[i].type );
        image->clearPixelData( eq::Frame::BUFFER_COLOR );
        image->disablePBO();

        // read
        clock.reset();
        image->startReadback( eq::Frame::BUFFER_COLOR, pvp, eq::Zoom(), 
                              glObjects );
        image->syncReadback();
        event.msec = clock.getTimef();

        GLenum error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );
        config->sendEvent( event );

        _saveImage( image, _enums[i].typeString, _enums[i].formatString,
                    "formats" );

        if( error != GL_NO_ERROR )
            continue;

        // PBO readback
        event.data.type = ConfigEvent::READBACK_PBO;
        image->clearPixelData( eq::Frame::BUFFER_COLOR );
        image->enablePBO();
        
        clock.reset();
        image->startReadback( eq::Frame::BUFFER_COLOR, pvp, eq::Zoom(),
                              glObjects );
        image->syncReadback();
        event.msec = clock.getTimef();
        
        error = glGetError();
        if( error != GL_NO_ERROR )
            event.msec = - static_cast<float>( error );
        config->sendEvent( event );
        
        _saveImage( image, _enums[i].typeString, _enums[i].formatString,
                    "formats_PBO" );

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
    }
}

void Channel::_testTiledOperations()
{
    //----- setup constant data
    const eq::ImageVector& images = _frame.getImages();
    EQASSERT( images[0] );

    eq::Config*              config = getConfig();
    const eq::PixelViewport& pvp    = getPixelViewport();
    const eq::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x() = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();

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
        snprintf( event.formatType, 64, "%d depth tiles", tiles+1 ); 

        event.msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];
            image->disablePBO();
            image->setFormat( eq::Frame::BUFFER_COLOR, GL_DEPTH_COMPONENT );
            image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_INT );
            image->clearPixelData( eq::Frame::BUFFER_COLOR );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_COLOR, subPVP, eq::Zoom(),
                                  glObjects );
            image->syncReadback();
            event.msec += clock.getTimef();
        }

        config->sendEvent( event );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage( images[j],
                            "GL_DEPTH_COMPONENT","GL_UNSIGNED_INT","tiles" );


        event.data.type = ConfigEvent::READBACK_PBO;

        for( unsigned j = 0; j <= tiles; ++j )
        {
            eq::Image* image = images[ j ];
            image->enablePBO();
            image->setFormat( eq::Frame::BUFFER_COLOR, GL_DEPTH_COMPONENT );
            image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_INT );
            image->clearPixelData( eq::Frame::BUFFER_COLOR );
        }
        clock.reset();
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            images[ j ]->startReadback( eq::Frame::BUFFER_COLOR, subPVP,
                                        eq::Zoom(), glObjects );
        }

        for( unsigned j = 0; j <= tiles; ++j )
            images[ j ]->syncReadback();

        event.msec = clock.getTimef();
        config->sendEvent( event );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
              _saveImage(images[j],
                            "GL_DEPTH_COMPONENT","GL_UNSIGNED_INT","tiles_PBO");


        //---- readback of 'tiles' color images
        event.data.type = ConfigEvent::READBACK;
        snprintf( event.formatType, 64, "%d color tiles", tiles+1 );

        event.msec = 0;
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            eq::Image* image = images[ j ];
            image->disablePBO();
            image->setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
            image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
            image->clearPixelData( eq::Frame::BUFFER_COLOR );

            clock.reset();
            image->startReadback( eq::Frame::BUFFER_COLOR, subPVP, eq::Zoom(),
                                  glObjects );
            image->syncReadback();
            event.msec += clock.getTimef();
        }

        config->sendEvent( event );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage( images[j],"GL_BGRA","GL_UNSIGNED_BYTE","tiles" );


        event.data.type = ConfigEvent::READBACK_PBO;

        for( unsigned j = 0; j <= tiles; ++j )
        {
            eq::Image* image = images[ j ];
            image->enablePBO();
            image->setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
            image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
            image->clearPixelData( eq::Frame::BUFFER_COLOR );
        }
        clock.reset();
        for( unsigned j = 0; j <= tiles; ++j )
        {
            subPVP.y = pvp.y + j * subPVP.h;
            images[ j ]->startReadback( eq::Frame::BUFFER_COLOR, subPVP,
                                        eq::Zoom(), glObjects );
        }

        for( unsigned j = 0; j <= tiles; ++j )
            images[ j ]->syncReadback();

        event.msec = clock.getTimef();
        config->sendEvent( event );

        if( tiles == NUM_IMAGES-1 )
            for( unsigned j = 0; j <= tiles; ++j )
                _saveImage(images[j],"GL_BGRA","GL_UNSIGNED_BYTE","tiles_PBO" );


        //---- benchmark assembly operations
        subPVP.y = pvp.y + tiles * subPVP.h;

        eq::Compositor::ImageOp op;
        op.channel = this;
        op.buffers = eq::Frame::BUFFER_COLOR | eq::Frame::BUFFER_DEPTH;
        op.offset  = offset;

        // fixed-function
        event.data.type = ConfigEvent::ASSEMBLE;
        snprintf( event.formatType, 64, 
                  "Tiled assembly (GL1.1) of %d images", tiles+1 ); 

        clock.reset();
        for( unsigned j = 0; j <= tiles; ++j )
            eq::Compositor::assembleImage( images[j], op );

        event.msec = clock.getTimef();
        config->sendEvent( event );

        // CPU
        snprintf( event.formatType, 64,
                  "Tiled assembly (CPU)   of %d images", tiles+1 ); 

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
    const eq::Vector2i     offset( pvp.x, pvp.y );

    ConfigEvent event = _createConfigEvent();
    event.area.x() = pvp.w;

    Clock                      clock;
    eq::Window::ObjectManager* glObjects = getObjectManager();


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
        image->setFormat( eq::Frame::BUFFER_COLOR, GL_BGRA );
        image->setType(   eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );
        image->setFormat( eq::Frame::BUFFER_DEPTH, GL_DEPTH_COMPONENT );
        image->setType(   eq::Frame::BUFFER_DEPTH, GL_UNSIGNED_INT );
        image->clearPixelData( eq::Frame::BUFFER_COLOR );
        image->clearPixelData( eq::Frame::BUFFER_DEPTH );

        image->startReadback( eq::Frame::BUFFER_COLOR | 
                              eq::Frame::BUFFER_DEPTH, pvp, eq::Zoom(),
                              glObjects );
        image->syncReadback();

        if( i == NUM_IMAGES-1 )
            _saveImage( image,"GL_BGRA","GL_UNSIGNED_BYTE","depthAssemble" );

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

void Channel::_saveImage( const eq::Image* image,
                          const char*      type,
                          const char*      format,
                          const char*      info    )
{
    return;

    static uint32_t counter = 0;
    ostringstream stringstream;
    stringstream << "Image_" << ++counter << "_"
                 << type << "_" << format << "_" << info;
    image->writeImages( stringstream.str( ));
}


void Channel::_draw( const uint32_t spin )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    eq::Channel::frameDraw( spin );

    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );

#ifdef _0
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
    glRotatef( static_cast< float >( spin + 3 ) * 10, 1.0f, 0.5f, 0.25f );

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

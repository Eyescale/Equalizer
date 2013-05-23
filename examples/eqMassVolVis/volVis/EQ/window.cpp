
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "window.h"

#include "../renderer/model.h"

#include "pipe.h"
#include "error.h"
#include "channel.h"

namespace massVolVis
{


bool Window::configInit( const eq::uint128_t& initId )
{
    // Enforce alpha channel, since we need one for rendering
    setIAttribute( IATTR_PLANES_ALPHA, 8 );

    return eq::Window::configInit( initId );
}


bool Window::configInitGL( const eq::uint128_t& initId )
{
    if( !GLEW_ARB_shader_objects )
    {
        setError( ERROR_VOLVIS_ARB_SHADER_OBJECTS_MISSING );
        return false;
    }
    if( !GLEW_EXT_blend_func_separate )
    {
        setError( ERROR_VOLVIS_EXT_BLEND_FUNC_SEPARATE_MISSING );
        return false;
    }
    if( !GLEW_ARB_multitexture )
    {
        setError( ERROR_VOLVIS_ARB_MULTITEXTURE_MISSING );
        return false;
    }

    if( !GLEW_NV_texture_barrier )
    {
        setError( ERROR_VOLVIS_NV_TEXTURE_BARRIER_MISSING );
        return false;
    }

    glEnable( GL_SCISSOR_TEST ); // needed to constrain channel viewport

    glClear( GL_COLOR_BUFFER_BIT );
    swapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDisable( GL_DEPTH_TEST );

    if( !eq::Window::configInitGL( initId ))
        return false;

//    _loadLogo();

    return true;
}

void Window::frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    eq::Window::frameStart( frameId, frameNumber );

    Pipe* pipe = static_cast<Pipe*>( getPipe( ));
    pipe->checkRenderingParameters( this, frameNumber );

    ModelSPtr model = pipe->getModel();
    if( model )
        model->glewSetContext( glewGetContext( ));
}


namespace
{
#ifdef EQ_RELEASE
#  ifdef _WIN32 // final INSTALL_DIR is not known at compile time
const std::string _logoTextureName =
                              std::string( "../share/Equalizer/data/logo.rgb" );
#  else
const std::string _logoTextureName = std::string( EQ_INSTALL_DIR ) +
                                 std::string( "share/Equalizer/data/logo.rgb" );
#  endif
#else
const std::string _logoTextureName = std::string( EQ_SOURCE_DIR ) +
                                      std::string( "/massVolVis/volVis/logo.rgb" );
#endif
}

void Window::_loadLogo()
{
    LBWARN << "max texture size: " << GL_MAX_TEXTURE_SIZE << std::endl;
    if( !GLEW_ARB_texture_rectangle )
    {
        LBWARN << "Can't load overlay logo, GL_ARB_texture_rectangle not "
               << "available" << std::endl;
        return;
    }

    eq::Window::ObjectManager* om = getObjectManager();
    _logoTexture = om->getEqTexture( _logoTextureName.c_str( ));
    if( _logoTexture )
        return;

    eq::Image image;
    if( !image.readImage( _logoTextureName, eq::Frame::BUFFER_COLOR ))
    {
        LBWARN << "Can't load overlay logo " << _logoTextureName << std::endl;
        return;
    }

    _logoTexture = om->newEqTexture( _logoTextureName.c_str(), GL_TEXTURE_RECTANGLE_ARB );

    LBASSERT( _logoTexture );

    image.upload(eq::Frame::BUFFER_COLOR, _logoTexture, eq::Vector2i::ZERO, om);
    image.deleteGLObjects( om );
    EQVERB << "Created logo texture of size " << _logoTexture->getWidth() << "x"
           << _logoTexture->getHeight() << std::endl;
}


void Window::swapBuffers()
{
    const Pipe*         pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData&    frameData = pipe->getFrameData();
    const eq::Channels& channels  = getChannels();

    if( frameData.useStatistics() && !channels.empty( ))
    {
        EQ_GL_CALL( channels.back()->drawStatistics( ));

        Channel* channel = dynamic_cast<Channel*>( channels.back() );
        if( channel )
            channel->drawFPS( getPixelViewport(), getFPS() );
    }

    eq::Window::swapBuffers();
}

}//namespace massVolVis

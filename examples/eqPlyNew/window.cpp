
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "pipe.h"
#include "node.h"

using namespace std;

namespace eqPly
{

bool Window::configInit( const uint32_t initID )
{
    if( !eq::Window::configInit( initID ))
        return false;

    eq::Pipe*  pipe        = getPipe();
    Window*    firstWindow = static_cast< Window* >( pipe->getWindow( 0 ));

    EQASSERT( !_state );

    if( firstWindow == this )
    {
        _state = new VertexBufferState( getGLFunctions( ));
        _loadLogo();

        const Node*     node     = static_cast< const Node* >( getNode( ));
        const InitData& initData = node->getInitData();

        if( initData.useVBOs() )
        {
            const eq::GLFunctions* glFunctions = getGLFunctions();
            // Check if all VBO funcs available, else leave DISPLAY_LIST_MODE on
            if( glFunctions->hasGenBuffers() && glFunctions->hasBindBuffer() &&
                glFunctions->hasBufferData() && glFunctions->hasDeleteBuffers())
            {
                _state->setRenderMode( mesh::BUFFER_OBJECT_MODE );
                EQINFO << "VBO rendering enabled" << endl;
            }
            else
                EQWARN << "VBO function pointers missing, using display lists" 
                       << endl;
        }
    }
    else
    {
        _state       = firstWindow->_state;
        _logoTexture = firstWindow->_logoTexture;
        _logoSize    = firstWindow->_logoSize;
    }

    if( !_state ) // happens if first window failed to initialize
        return false;
    
    return true;
}

bool Window::configExit()
{
    if( _state.isValid() && _state->getRefCount() == 1 )
        _state->deleteAll();

    _state = 0;
    return eq::Window::configExit();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    EQASSERT( _state->getTexture( _logoTextureName ) == 
              VertexBufferState::FAILED );

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "examples/eqPly/logo.rgb", eq::Frame::BUFFER_COLOR ))
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << endl;
        return;
    }

    _logoTexture = _state->newTexture( _logoTextureName );
    EQASSERT( _logoTexture != VertexBufferState::FAILED );

    const eq::PixelViewport& pvp = image.getPixelViewport();
    _logoSize.x = pvp.w;
    _logoSize.y = pvp.h;

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _logoTexture );
    glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                  image.getFormat( eq::Frame::BUFFER_COLOR ),
                  _logoSize.x, _logoSize.y, 0,
                  image.getFormat( eq::Frame::BUFFER_COLOR ), 
                  image.getType( eq::Frame::BUFFER_COLOR ),
                  image.getPixelData( eq::Frame::BUFFER_COLOR ));

    EQINFO << "Created logo texture of size " << _logoSize << endl;
}

}

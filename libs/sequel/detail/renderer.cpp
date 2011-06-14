
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "renderer.h"

#include "channel.h"
#include "pipe.h"
#include "window.h"

#include <eq/sequel/renderer.h>

namespace seq
{
namespace detail
{

Renderer::Renderer( seq::Renderer* parent )
        : _renderer( parent )
        , _pipe( 0 )
        , _channel( 0 )
{}

Renderer::~Renderer()
{
    EQASSERT( !_pipe );
    EQASSERT( !_channel );
}

co::Object* Renderer::getFrameData()
{
    return _pipe->getFrameData();
}

Window* Renderer::getWindow()
{
    EQASSERT( _channel );
    if( !_channel )
        return 0;

    return static_cast< Window* >( _channel->getWindow( ));
}

const GLEWContext* Renderer::glewGetContext() const
{
    EQASSERT( _channel );
    return _channel ? _channel->glewGetContext() : 0 ;
}

bool Renderer::initGL()
{
    Window* window = getWindow();
    return window ? window->initGL() : false;
}

bool Renderer::exitGL()
{
    Window* window = getWindow();
    return window ? window->exitGL() : false;
}

void Renderer::applyRenderContext()
{
    EQASSERT( _channel );
    if( _channel )
        _channel->applyRenderContext();
}

}
}

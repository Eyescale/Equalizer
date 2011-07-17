
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

#include <sequel/renderer.h>

namespace seq
{
namespace detail
{

Renderer::Renderer( seq::Renderer* parent )
        : _renderer( parent )
        , _glewContext( 0 )
        , _pipe( 0 )
        , _window( 0 )
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

const Frustumf& Renderer::getFrustum() const
{
    EQASSERT( _channel );
    return _channel ? _channel->getFrustum() : Frustumf::DEFAULT;
}

const Matrix4f& Renderer::getViewMatrix() const
{
    EQASSERT( _channel );
    return _channel ? _channel->getViewMatrix() : Matrix4f::IDENTITY;
}

const Matrix4f& Renderer::getModelMatrix() const
{
    EQASSERT( _channel );
    return _channel ? _channel->getModelMatrix() : Matrix4f::IDENTITY;
}

bool Renderer::initGL()
{
    return _window ? _window->initGL() : false;
}

bool Renderer::exitGL()
{
    return _window ? _window->exitGL() : false;
}

void Renderer::applyRenderContext()
{
    EQASSERT( _channel );
    if( _channel )
        _channel->applyRenderContext();
}

void Renderer::applyModelMatrix()
{
    EQASSERT( _channel );
    if( _channel )
        _channel->applyModelMatrix();
}

}
}


/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.ch>
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

#include <seq/renderer.h>

namespace seq
{
namespace detail
{
static const RenderContext dummyContext;

Renderer::Renderer()
        : _glewContext( 0 )
        , _pipe( 0 )
        , _window( 0 )
        , _channel( 0 )
{}

Renderer::~Renderer()
{
    LBASSERT( !_pipe );
    LBASSERT( !_channel );
}

co::Object* Renderer::getFrameData()
{
    return _pipe->getFrameData();
}

const Frustumf& Renderer::getFrustum() const
{
    LBASSERT( _channel );
    return _channel ? _channel->getFrustum() : Frustumf::DEFAULT;
}

const Matrix4f& Renderer::getViewMatrix() const
{
    LBASSERT( _channel );
    return _channel ? _channel->getViewMatrix() : Matrix4f::IDENTITY;
}

const Matrix4f& Renderer::getModelMatrix() const
{
    LBASSERT( _channel );
    return _channel ? _channel->getModelMatrix() : Matrix4f::IDENTITY;
}

bool Renderer::useOrtho() const
{
    LBASSERT( _channel );
    return _channel ? _channel->useOrtho() : false;
}

void Renderer::setNearFar( const float nearPlane, const float farPlane )
{
    LBASSERT( _channel );
    if( _channel )
        _channel->setNearFar( nearPlane, farPlane );
}

void Renderer::setWindow( Window* window )
{
    _window = window;
    _glewContext = window ? window->glewGetContext() : 0;
}

void Renderer::setChannel( Channel* channel )
{
    _channel = channel;
    _glewContext = channel ? channel->glewGetContext() : 0;
}

bool Renderer::initContext()
{
    return _window ? _window->initContext() : false;
}

bool Renderer::exitContext()
{
    return _window ? _window->exitContext() : false;
}

void Renderer::clear()
{
    LBASSERT( _channel );
    if( _channel )
        _channel->clear();
}

void Renderer::applyRenderContext()
{
    LBASSERT( _channel );
    if( _channel )
        _channel->applyRenderContext();
}

const RenderContext& Renderer::getRenderContext() const
{
    LBASSERT( _channel );
    if( _channel )
        return _channel->getRenderContext();
    return dummyContext;
}

void Renderer::applyModelMatrix()
{
    LBASSERT( _channel );
    if( _channel )
        _channel->applyModelMatrix();
}

}
}

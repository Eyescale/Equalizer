
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

#include "application.h"
#include "viewData.h"
#include "detail/renderer.h"

namespace seq
{
Renderer::Renderer( Application& application )
        : _app( application )
{
    _impl = new detail::Renderer( this );
}

Renderer::~Renderer()
{
    delete _impl;
    _impl = 0;
}

co::Object* Renderer::getFrameData()
{
    return _impl->getFrameData();
}

const GLEWContext* Renderer::glewGetContext() const
{
    return _impl->glewGetContext();
}

ViewData* Renderer::createViewData()
{
    return new ViewData;
}

void Renderer::destroyViewData( ViewData* viewData )
{
    delete viewData;
}

const Frustumf& Renderer::getFrustum() const
{
    return _impl->getFrustum();
}

const Matrix4f& Renderer::getViewMatrix() const
{
    return _impl->getViewMatrix();
}

const Matrix4f& Renderer::getModelMatrix() const
{
    return _impl->getModelMatrix();
}

bool Renderer::initContext( co::Object* initData )
{
    return _impl->initContext();
}

bool Renderer::exitContext()
{
    return _impl->exitContext();
}

void Renderer::clear( co::Object* frameData )
{
    _impl->clear();
}

void Renderer::applyRenderContext()
{
    _impl->applyRenderContext();
}

const RenderContext& Renderer::getRenderContext() const
{
    return _impl->getRenderContext();
}

void Renderer::applyModelMatrix()
{
    _impl->applyModelMatrix();
}

co::Object* Renderer::createObject( const uint32_t type )
{
    return _app.createObject( type );
}

void Renderer::destroyObject( co::Object* object, const uint32_t type )
{
    _app.destroyObject( object, type );
}

}

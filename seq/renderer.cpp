
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Petros Kataras <petroskataras@gmail.com>
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
#include "detail/objectMap.h"
#include "detail/renderer.h"
#include "detail/window.h"

namespace seq
{
Renderer::Renderer( Application& application )
    : _impl( new detail::Renderer )
    , app_( application )
{
}

Renderer::~Renderer()
{
    delete _impl;
}

co::Object* Renderer::getFrameData()
{
    return _impl->getFrameData();
}

const ObjectManager& Renderer::getObjectManager() const
{
    return _impl->getObjectManager();
}

ObjectManager& Renderer::getObjectManager()
{
    return _impl->getObjectManager();
}

const GLEWContext* Renderer::glewGetContext() const
{
    return _impl->glewGetContext();
}

const ViewData* Renderer::getViewData() const
{
    return _impl->getViewData();
}

ViewData* Renderer::createViewData( View& view )
{
    return new ViewData( view );
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

const PixelViewport& Renderer::getPixelViewport() const
{
    return _impl->getPixelViewport();
}

uint32_t Renderer::getWindowID() const
{
    const eq::Window* window = _impl->getWindow();
    return window ? window->getSerial() : CO_INSTANCE_INVALID;
}

bool Renderer::initContext( co::Object* /*initData*/ )
{
    return _impl->initContext();
}

bool Renderer::exitContext()
{
    return _impl->exitContext();
}

void Renderer::clear( co::Object* /*frameData*/ )
{
    _impl->clear();
}

void Renderer::updateNearFar( const Vector4f& boundingSphere )
{
    const Matrix4f& view = getViewMatrix();
    const Matrix4f& viewInv = view.inverse();
    const Vector3f& zero  = viewInv * Vector3f::ZERO;
    Vector3f        front = viewInv * Vector3f( 0.0f, 0.0f, -1.0f );
    front -= zero;
    front.normalize();
    front *= boundingSphere.w();

    const Vector3f& translation = getModelMatrix().getTranslation();
    const Vector3f& center = translation -
                             boundingSphere.get_sub_vector< 3, 0 >();
    const Vector3f& nearPoint  = view * ( center - front );
    const Vector3f& farPoint   = view * ( center + front );

    if( _impl->useOrtho( ))
    {
        LBASSERTINFO( fabs( farPoint.z() - nearPoint.z() ) >
                      std::numeric_limits< float >::epsilon(),
                      nearPoint << " == " << farPoint );
        setNearFar( -nearPoint.z(), -farPoint.z() );
    }
    else
    {
        // estimate minimal value of near plane based on frustum size
        const eq::Frustumf& frustum = _impl->getFrustum();
        const float width  = fabs( frustum.right() - frustum.left() );
        const float height = fabs( frustum.top() - frustum.bottom() );
        const float size   = LB_MIN( width, height );
        const float minNear = frustum.nearPlane() / size * .001f;

        const float zNear = LB_MAX( minNear, -nearPoint.z() );
        const float zFar  = LB_MAX( zNear * 2.f, -farPoint.z() );

        setNearFar( zNear, zFar );
    }
}

void Renderer::setNearFar( const float nearPlane, const float farPlane )
{
    _impl->setNearFar( nearPlane, farPlane );
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

void Renderer::applyScreenFrustum()
{
    _impl->applyScreenFrustum();
}

void Renderer::applyPerspectiveFrustum()
{
    _impl->applyPerspectiveFrustum();
}

co::Object* Renderer::createObject( const uint32_t type )
{
    return app_.createObject( type );
}

void Renderer::destroyObject( co::Object* object, const uint32_t type )
{
    app_.destroyObject( object, type );
}

co::Object* Renderer::mapObject( const uint128_t& identifier,
                                 co::Object* instance )
{
   return _impl->mapObject( identifier, instance );
}

bool Renderer::unmap( co::Object* object )
{
   return _impl->unmap( object );
}


}

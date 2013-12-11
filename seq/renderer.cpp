
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

#include "application.h"
#include "viewData.h"
#include "detail/renderer.h"

namespace seq
{
Renderer::Renderer( Application& application )
    : impl_( new detail::Renderer )
    , app_( application )
{
}

Renderer::~Renderer()
{
    delete impl_;
}

co::Object* Renderer::getFrameData()
{
    return impl_->getFrameData();
}

const GLEWContext* Renderer::glewGetContext() const
{
    return impl_->glewGetContext();
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
    return impl_->getFrustum();
}

const Matrix4f& Renderer::getViewMatrix() const
{
    return impl_->getViewMatrix();
}

const Matrix4f& Renderer::getModelMatrix() const
{
    return impl_->getModelMatrix();
}

bool Renderer::initContext( co::Object* /*initData*/ )
{
    return impl_->initContext();
}

bool Renderer::exitContext()
{
    return impl_->exitContext();
}

void Renderer::clear( co::Object* /*frameData*/ )
{
    impl_->clear();
}

void Renderer::updateNearFar( const Vector4f& boundingSphere )
{
    const Matrix4f& view = getViewMatrix();
    Matrix4f viewInv;
    compute_inverse( view, viewInv );

    const Vector3f& zero  = viewInv * Vector3f::ZERO;
    Vector3f        front = viewInv * Vector3f( 0.0f, 0.0f, -1.0f );
    front -= zero;
    front.normalize();
    front *= boundingSphere.w();

    const Vector3f& translation = getModelMatrix().get_translation();
    const Vector3f& center = translation - boundingSphere.get_sub_vector< 3 >();
    const Vector3f& nearPoint  = view * ( center - front );
    const Vector3f& farPoint   = view * ( center + front );

    if( impl_->useOrtho( ))
    {
        LBASSERTINFO( fabs( farPoint.z() - nearPoint.z() ) >
                      std::numeric_limits< float >::epsilon(),
                      nearPoint << " == " << farPoint );
        setNearFar( -nearPoint.z(), -farPoint.z() );
    }
    else
    {
        // estimate minimal value of near plane based on frustum size
        const eq::Frustumf& frustum = impl_->getFrustum();
        const float width  = fabs( frustum.right() - frustum.left() );
        const float height = fabs( frustum.top() - frustum.bottom() );
        const float size   = LB_MIN( width, height );
        const float minNear = frustum.near_plane() / size * .001f;

        const float zNear = LB_MAX( minNear, -nearPoint.z() );
        const float zFar  = LB_MAX( zNear * 2.f, -farPoint.z() );

        setNearFar( zNear, zFar );
    }
}

void Renderer::setNearFar( const float nearPlane, const float farPlane )
{
    impl_->setNearFar( nearPlane, farPlane );
}

void Renderer::applyRenderContext()
{
    impl_->applyRenderContext();
}

const RenderContext& Renderer::getRenderContext() const
{
    return impl_->getRenderContext();
}

void Renderer::applyModelMatrix()
{
    impl_->applyModelMatrix();
}

co::Object* Renderer::createObject( const uint32_t type )
{
    return app_.createObject( type );
}

void Renderer::destroyObject( co::Object* object, const uint32_t type )
{
    app_.destroyObject( object, type );
}
}

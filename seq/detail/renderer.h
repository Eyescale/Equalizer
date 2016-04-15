
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_DETAIL_RENDERER_H
#define EQSEQUEL_DETAIL_RENDERER_H

#include <seq/types.h>

namespace seq
{
namespace detail
{
/** The internal implementation for the renderer. */
class Renderer
{
public:
    Renderer();
    ~Renderer();

    /** @name Data Access. */
    //@{
    co::Object* getFrameData();
    const GLEWContext* glewGetContext() const { return _glewContext; }

    const ObjectManager& getObjectManager() const;
    ObjectManager& getObjectManager();

    const ViewData* getViewData() const;

    const RenderContext& getRenderContext() const;
    const Frustumf& getFrustum() const;
    const Matrix4f& getViewMatrix() const;
    const Matrix4f& getModelMatrix() const;
    const PixelViewport& getPixelViewport() const;

    bool useOrtho() const;
    void setNearFar( const float nearPlane, const float farPlane );
    //@}

    /** @name Current context. */
    //@{
    void setPipe( Pipe* pipe ) { _pipe = pipe; }
    void setWindow( Window* window );
    void setChannel( Channel* channel );

    const Window* getWindow() const { return _window; }
    //@}

    /** @name Operations. */
    //@{
    bool initContext();
    bool exitContext();

    void clear();

    void applyRenderContext();
    void applyModelMatrix();

    void applyScreenFrustum();
    void applyPerspectiveFrustum();
    //@}

    /** @name Distributed Object API. */
    //@{
    co::Object* mapObject( const uint128_t& identifier,
                           co::Object* instance = 0 );
    bool unmap( co::Object* object );

private:
    const GLEWContext* _glewContext;
    Pipe* _pipe;
    Window* _window;
    Channel* _channel;
};
}
}

#endif // EQSEQUEL_DETAIL_RENDERER_H

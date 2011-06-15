
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

#ifndef EQSEQUEL_DETAIL_RENDERER_H
#define EQSEQUEL_DETAIL_RENDERER_H

#include <eq/sequel/types.h>
#include <eq/nodeFactory.h> // base class

namespace seq
{
namespace detail
{
    /** The internal implementation for the renderer. */
    class Renderer
    {
    public:
        Renderer( seq::Renderer* parent );
        ~Renderer();

        /** @name Data Access. */
        //@{
        co::Object* getFrameData();
        void setGLEWContext( const GLEWContext* ctx ) { _glewContext = ctx; }
        const GLEWContext* glewGetContext() const { return _glewContext; }

        const Frustumf& getFrustum() const;
        const Matrix4f& getViewMatrix() const;
        const Matrix4f& getModelMatrix() const;
        //@}

        /** @name Current context. */
        //@{
        void setPipe( Pipe* pipe ) { _pipe = pipe; }
        void setWindow( Window* window ) { _window = window; }
        void setChannel( Channel* channel ) { _channel = channel; }
        //@}

        /** @name Operations. */
        //@{
        bool initGL();
        bool exitGL();
        void applyRenderContext();
        void applyModelMatrix();
        //@}

    private:
        seq::Renderer* const _renderer;
        const GLEWContext* _glewContext;
        Pipe* _pipe;
        Window* _window;
        Channel* _channel;
    };
}
}

#endif // EQSEQUEL_DETAIL_RENDERER_H


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

        /** @name Current context. */
        //@{
        void setChannel( Channel* channel ) { _channel = channel; }
        //@}

        /** @name Operations. */
        //@{
        void applyRenderContext();
        //@}

    private:
        seq::Renderer* const _renderer;
        Channel* _channel;
    };
}
}

#endif // EQSEQUEL_DETAIL_RENDERER_H

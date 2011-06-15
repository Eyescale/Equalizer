
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

#ifndef EQSEQUEL_DETAIL_WINDOW_H
#define EQSEQUEL_DETAIL_WINDOW_H

#include <eq/sequel/types.h>

#include <eq/window.h> // base class

namespace seq
{
namespace detail
{
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent );

        Config* getConfig();
        Pipe* getPipe();
        seq::Renderer* getRenderer();
        detail::Renderer* getRendererImpl();

        /** @name Operations. */
        //@{
        virtual void frameStart( const uint128_t& frameID,
                                 const uint32_t frameNumber );
        virtual void frameFinish( const uint128_t& frameID,
                                  const uint32_t frameNumber );
        virtual bool configInitGL( const uint128_t& initID );
        virtual bool configExitGL();

        virtual bool initGL() { return eq::Window::configInitGL( 0 ); }
        virtual bool exitGL() { return eq::Window::configExitGL(); }
        //@}

    protected:
        virtual ~Window();

    private:
    };
}
}

#endif // EQSEQUEL_DETAIL_WINDOW_H

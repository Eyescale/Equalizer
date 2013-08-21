
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

#ifndef EQSEQUEL_DETAIL_WINDOW_H
#define EQSEQUEL_DETAIL_WINDOW_H

#include <seq/types.h>
#include <eq/client/window.h> // base class

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

        bool initContext() { return eq::Window::configInitGL( uint128_t( )); }
        bool exitContext() { return eq::Window::configExitGL(); }
        //@}

    protected:
        virtual ~Window();

    private:
    };
}
}

#endif // EQSEQUEL_DETAIL_WINDOW_H

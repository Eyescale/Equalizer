
/* Copyright (c) 2011-2017, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_DETAIL_WINDOW_H
#define EQSEQUEL_DETAIL_WINDOW_H

#include <seq/types.h>
#include <eq/window.h> // base class

namespace seq
{
namespace detail
{
class Window : public eq::Window
{
public:
    explicit Window( eq::Pipe* parent );

    Config* getConfig();
    Pipe* getPipe();
    seq::Renderer* getRenderer();
    detail::Renderer* getRendererImpl();

    /** @name Operations. */
    //@{
    void frameStart( const uint128_t& frameID, const uint32_t frameNumber )
        final;
    void frameFinish( const uint128_t& frameID, const uint32_t frameNumber )
        final;
    bool configInitGL( const uint128_t& initID ) final;
    bool configExitGL() final;

    bool processEvent( eq::EventType type ) final;
    bool processEvent( eq::EventType type, SizeEvent& event ) final;
    bool processEvent( eq::EventType type, PointerEvent& event ) final;
    bool processEvent( eq::EventType type, KeyEvent& event ) final;
    bool processEvent( AxisEvent& event ) final;
    bool processEvent( ButtonEvent& event ) final;

    bool initContext() { return eq::Window::configInitGL( uint128_t( )); }
    bool exitContext() { return eq::Window::configExitGL(); }
    //@}

private:
    virtual ~Window();

    template< class E > bool _processEvent( eq::EventType type, E& event );
    template< class E > bool _processEvent( E& event );
};
}
}

#endif // EQSEQUEL_DETAIL_WINDOW_H

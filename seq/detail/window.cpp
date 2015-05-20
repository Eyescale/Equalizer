
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@eyescale.ch>
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

#include "window.h"

#include "config.h"
#include "pipe.h"
#include "renderer.h"

#include <seq/renderer.h>
#include <eq/util/objectManager.h>
#include <eq/fabric/event.h>

namespace seq
{
namespace detail
{

Window::Window( eq::Pipe* parent )
    : eq::Window( parent )
{}

Window::~Window()
{
}

Config* Window::getConfig()
{
    return static_cast< Config* >( eq::Window::getConfig( ));
}

Pipe* Window::getPipe()
{
    return static_cast< Pipe* >( eq::Window::getPipe( ));
}

seq::Renderer* Window::getRenderer()
{
    return getPipe()->getRenderer();
}

detail::Renderer* Window::getRendererImpl()
{
    return getPipe()->getRendererImpl();
}

void Window::frameStart( const uint128_t& frameID, const uint32_t frameNumber )
{
    getRendererImpl()->setWindow( this );
    eq::Window::frameStart( frameID, frameNumber );
}

void Window::frameFinish( const uint128_t& frameID, const uint32_t frameNumber)
{
    getRendererImpl()->setWindow( 0 );
    eq::Window::frameFinish( frameID, frameNumber );
}

bool Window::configInitGL( const uint128_t& )
{
    Renderer* rendererImpl = getRendererImpl();
    rendererImpl->setWindow( this );

    co::Object* initData = getConfig()->getInitData();
    seq::Renderer* const renderer = getRenderer();
    const bool first = !getObjectManager().isShared();

    if( first && !renderer->init( initData ))
    {
        rendererImpl->setWindow( 0 );
        return false;
    }
    const bool ret = renderer->initContext( initData );

    rendererImpl->setWindow( 0 );
    return ret;
}

bool Window::configExitGL()
{
    Renderer* rendererImpl = getRendererImpl();
    rendererImpl->setWindow( this );
    seq::Renderer* const renderer = getRenderer();
    const bool last = !getObjectManager().isShared();

    bool ret = renderer->exitContext();
    if( last && !renderer->exit( ))
        ret = false;

    rendererImpl->setWindow( 0 );
    return ret;
}

bool Window::processEvent( const eq::Event& event )
{
    seq::Renderer* const renderer = getRenderer();
    if( renderer->processEvent( event ))
        return true;
    return eq::Window::processEvent( event );
}

}
}


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

#include "channel.h"

#include "pipe.h"
#include "renderer.h"
#include <eq/sequel/renderer.h>

namespace seq
{
namespace detail
{

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent ) 
{}

Channel::~Channel()
{
}

Pipe* Channel::getPipe()
{
    return static_cast< Pipe* >( eq::Channel::getPipe( ));
}

seq::Renderer* Channel::getRenderer()
{
    return getPipe()->getRenderer();
}

detail::Renderer* Channel::getRendererImpl()
{
    return getPipe()->getRendererImpl();
}

void Channel::frameStart( const uint128_t& frameID, const uint32_t frameNumber )
{
    getRendererImpl()->setChannel( this );
    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameFinish( const uint128_t& frameID, const uint32_t frameNumber)
{
    getRendererImpl()->setChannel( 0 );
    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameDraw( const uint128_t& frameID )
{
    seq::Renderer* const renderer = getRenderer();
    co::Object* const frameData = renderer->getFrameData();
    renderer->draw( frameData );
}

}
}

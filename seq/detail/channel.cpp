
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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
#include "view.h"

#include <seq/renderer.h>
#include <seq/viewData.h>
#include <eq/gl.h>

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

const View* Channel::getView() const
{
    return static_cast< const View* >( eq::Channel::getView( ));
}

const ViewData* Channel::getViewData() const
{
    const View* view = getView();
    return view ? view->getViewData() : 0;
}

seq::Renderer* Channel::getRenderer()
{
    return getPipe()->getRenderer();
}

detail::Renderer* Channel::getRendererImpl()
{
    return getPipe()->getRendererImpl();
}

const Matrix4f& Channel::getModelMatrix() const
{
    const ViewData* data = getViewData();
    LBASSERT( data );
    static const Matrix4f identity;
    if( !data )
        return identity;

    return data->getModelMatrix();
}

bool Channel::useOrtho() const
{
    const ViewData* data = getViewData();
    LBASSERT( data );
    if( !data )
        return false;

    return data->useOrtho();
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

void Channel::frameClear( const uint128_t& )
{
    seq::Renderer* const renderer = getRenderer();
    co::Object* const frameData = renderer->getFrameData();
    renderer->clear( frameData );
}

void Channel::frameDraw( const uint128_t& )
{
    seq::Renderer* const renderer = getRenderer();
    co::Object* const frameData = renderer->getFrameData();
    renderer->draw( frameData );
}

void Channel::applyModelMatrix()
{
    EQ_GL_CALL( glMultMatrixf( getModelMatrix().array ));
}

void Channel::frameViewFinish( const uint128_t& frameID )
{
    const ViewData* data = getViewData();
    LBASSERT( data );
    if( data && data->getStatistics( ))
    {
        applyBuffer();
        applyViewport();
        drawStatistics();
    }

    eq::Channel::frameViewFinish( frameID );
}

}
}


/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "proxy.h"
#include "eventHandler.h"

#include "../channel.h"
#include "../pipe.h"
#include "../view.h"
#include "../windowSystem.h"

#include <eq/fabric/drawableConfig.h>
#include <eq/util/objectManager.h>
#include <eq/util/texture.h>

#include <dc/Stream.h>
#include <GL/gl.h>

namespace eq
{
namespace dc
{
namespace detail
{
class Proxy : public boost::noncopyable
{
public:
    Proxy( eq::Channel* ch )
        : stream( 0 )
        , eventHandler( 0 )
        , channel( ch )
        , running( false )
        , texture( GL_TEXTURE_RECTANGLE_ARB,
                   channel->getObjectManager().glewGetContext( ))
    {
        const DrawableConfig& dc = channel->getDrawableConfig();
        if( dc.colorBits != 8 )
        {
            LBWARN << "Can only stream 8-bit RGBA framebuffers to "
                   << "DisplayCluster, got " << dc.colorBits << " color bits"
                   << std::endl;
            return;
        }

        const std::string& dcHost = channel->getView()->getDisplayCluster();
        stream = new ::dc::Stream( channel->getView()->getName(), dcHost );
        if( !stream->isConnected( ))
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }

        running = true;
    }

    ~Proxy()
    {
        delete eventHandler;
        delete stream;
    }

    void swapBuffer()
    {
        const PixelViewport& pvp = channel->getPixelViewport();
        const size_t newSize = pvp.w * pvp.h * 4;
        buffer.reserve(newSize);

        texture.copyFromFrameBuffer( GL_RGBA, pvp );
        // Needed as copyFromFrameBuffer only grows the texture!
        texture.resize( pvp.w, pvp.h );
        texture.download( buffer.getData() );

        const Viewport& vp = channel->getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::dc::ImageWrapper::swapYAxis( buffer.getData(), pvp.w, pvp.h, 4 );
        ::dc::ImageWrapper imageWrapper( buffer.getData(), pvp.w, pvp.h,
                                         ::dc::RGBA, offsX, offsY );

        running = stream->send( imageWrapper ) && stream->finishFrame();
    }

    ::dc::Stream* stream;
    EventHandler* eventHandler;
    eq::Channel* channel;
    lunchbox::Bufferb buffer;
    bool running;
    util::Texture texture;
};
}

Proxy::Proxy( Channel* channel )
    : _impl( new detail::Proxy( channel ))
{
}

Proxy::~Proxy()
{
    delete _impl;
}

void Proxy::swapBuffer()
{
    _impl->swapBuffer();

    if( !_impl->eventHandler && _impl->stream->registerForEvents( ))
    {
        _impl->eventHandler = new EventHandler( this );
        LBINFO << "Installed event handler for DisplayCluster" << std::endl;
    }
}

Channel* Proxy::getChannel()
{
    return _impl->channel;
}

int Proxy::getSocketDescriptor() const
{
    return _impl->stream->getDescriptor();
}

bool Proxy::hasNewEvent()
{
    return _impl->stream->hasEvent();
}

bool Proxy::isRunning() const
{
    return _impl->running;
}

void Proxy::stopRunning()
{
    _impl->running = false;
}

::dc::Event Proxy::getEvent() const
{
    return _impl->stream->getEvent();
}

}
}

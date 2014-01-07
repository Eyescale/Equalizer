
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
class Proxy
{
public:
    Proxy( eq::Channel* channel )
        : _dcStream( 0 )
        , _eventHandler( 0 )
        , _channel( channel )
        , _isRunning( false )
        , _texture( GL_TEXTURE_RECTANGLE_ARB, channel->getObjectManager().glewGetContext( ))
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
        _dcStream = new ::dc::Stream( _channel->getView()->getName(), dcHost);
        if( !_dcStream->isConnected( ))
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }

        _isRunning = true;
    }

    ~Proxy()
    {
        delete _eventHandler;

        delete _dcStream;
    }

    void swapBuffer()
    {
        const PixelViewport& pvp = _channel->getPixelViewport();
        const size_t newSize = pvp.w * pvp.h * 4;
        _buffer.reserve(newSize);

        _texture.copyFromFrameBuffer( GL_RGBA, pvp );
        _texture.resize( pvp.w, pvp.h ); // Needed as copyFromFrameBuffer only grows the texture!
        _texture.download( _buffer.getData() );

        const Viewport& vp = _channel->getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::dc::ImageWrapper::reorderGLImageData((void*)_buffer.getData(), pvp.w, pvp.h, 4);
        ::dc::ImageWrapper imageWrapper(_buffer.getData(), pvp.w, pvp.h, ::dc::RGBA, offsX, offsY);

        _isRunning = _dcStream->send(imageWrapper) && _dcStream->finishFrame();
    }

    ::dc::Stream* _dcStream;
    EventHandler* _eventHandler;
    eq::Channel* _channel;
    lunchbox::Buffer<unsigned char> _buffer;
    bool _isRunning;
    util::Texture _texture;
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

    if( !_impl->_eventHandler &&
            (_impl->_dcStream->isRegisteredForEvents() || _impl->_dcStream->registerForEvents()) &&
            _impl->_dcStream->hasEvent( ))
    {
        _impl->_eventHandler = new EventHandler( this );
        LBINFO << "Installed event handler for DisplayCluster" << std::endl;
    }
}

Channel* Proxy::getChannel()
{
    return _impl->_channel;
}

int Proxy::getSocketDescriptor() const
{
    return _impl->_dcStream->getDescriptor();
}

bool Proxy::hasNewEvent()
{
    return _impl->_dcStream->hasEvent();
}

bool Proxy::isRunning() const
{
    return _impl->_isRunning;
}

void Proxy::stopRunning()
{
    _impl->_isRunning = false;
}

::dc::Event Proxy::getEvent() const
{
    return _impl->_dcStream->getEvent();
}

}
}

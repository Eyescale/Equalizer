
/* Copyright (c) 2013-2014, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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
#include "../systemWindow.h"
#include "../view.h"
#include "../window.h"
#include "../windowSystem.h"

#include <eq/fabric/drawableConfig.h>
#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>
#include <eq/util/texture.h>

#include <lunchbox/buffer.h>
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
        : _stream( 0 )
        , _eventHandler( 0 )
        , _channel( ch )
        , _running( false )
        , _texture( 0 )
    {
        const DrawableConfig& dc = _channel->getDrawableConfig();
        if( dc.colorBits != 8 )
        {
            LBWARN << "Can only stream 8-bit RGB(A) framebuffers to "
                   << "DisplayCluster, got " << dc.colorBits << " color bits"
                   << std::endl;
            return;
        }

        const std::string& dcHost = _channel->getView()->getDisplayCluster();
        _stream = new ::dc::Stream( _channel->getView()->getName(), dcHost );
        if( !_stream->isConnected( ))
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }

        _running = true;
    }

    ~Proxy()
    {
        if( _texture )
            _texture->flush();

        delete _texture;
        delete _eventHandler;
        delete _stream;
    }

    void swapBuffer()
    {
        const PixelViewport& pvp = _channel->getPixelViewport();
        const SystemWindow* sysWindow = _channel->getWindow()->getSystemWindow();
        const util::FrameBufferObject* fbo = sysWindow->getFrameBufferObject();
        ::dc::PixelFormat pixelFormat = ::dc::RGB;
        size_t bytesPerPixel = 3;

        // OPT: use FBO texture directly to download
        if( fbo && fbo->getColorTextures().size() == 1 )
        {
            const util::Texture* texture = fbo->getColorTextures().front();
            if( texture->getFormat() == GL_RGBA )
            {
                pixelFormat = ::dc::RGBA;
                bytesPerPixel = 4;
            }

            const size_t newSize = pvp.w * pvp.h * bytesPerPixel;
            buffer.reserve( newSize );
            texture->download( buffer.getData( ));
        }
        else
        {
            if( !_texture )
                _texture = new util::Texture( GL_TEXTURE_RECTANGLE_ARB,
                                _channel->getObjectManager().glewGetContext( ));

            const size_t newSize = pvp.w * pvp.h * bytesPerPixel;
            buffer.reserve( newSize );

            _texture->copyFromFrameBuffer( GL_RGB, pvp );
            // Needed as copyFromFrameBuffer only grows the texture!
            _texture->resize( pvp.w, pvp.h );
            _texture->download( buffer.getData( ));
        }

        const Viewport& vp = _channel->getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::dc::ImageWrapper::swapYAxis( buffer.getData(), pvp.w, pvp.h,
                                       bytesPerPixel );
        ::dc::ImageWrapper imageWrapper( buffer.getData(), pvp.w, pvp.h,
                                         pixelFormat, offsX, offsY );
        imageWrapper.compressionPolicy = ::dc::COMPRESSION_ON;
        imageWrapper.compressionQuality = 100;

        _running = _stream->send( imageWrapper ) && _stream->finishFrame();
    }

    ::dc::Stream* _stream;
    EventHandler* _eventHandler;
    eq::Channel* _channel;
    lunchbox::Bufferb buffer;
    bool _running;
    util::Texture* _texture;
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

    if( !_impl->_eventHandler && _impl->_stream->registerForEvents( ))
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
    return _impl->_stream->getDescriptor();
}

bool Proxy::hasNewEvent() const
{
    return _impl->_stream->hasEvent();
}

bool Proxy::isRunning() const
{
    return _impl->_running;
}

void Proxy::stopRunning()
{
    _impl->_running = false;
}

::dc::Event Proxy::getEvent() const
{
    return _impl->_stream->getEvent();
}

}
}


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

::dc::Stream::Future make_ready_future( bool value )
{
    boost::promise< bool > promise;
    promise.set_value( value );
    return promise.get_future();
}

class Proxy : public boost::noncopyable
{
public:
    Proxy( eq::Channel* ch )
        : _stream( 0 )
        , _eventHandler( 0 )
        , _channel( ch )
        , _sendFuture( make_ready_future( false ))
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

        const std::string& dcHost =
               _channel->getView()->getSAttribute( View::SATTR_DISPLAYCLUSTER );
        const std::string& name =
             _channel->getView()->getSAttribute( View::SATTR_PIXELSTREAM_NAME );
        _stream = new ::dc::Stream( name, dcHost );
        if( !_stream->isConnected( ))
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }

        _running = true;
        _sendFuture = make_ready_future( true );
    }

    ~Proxy()
    {
        // wait for completion of previous send
        _sendFuture.wait();

        if( _texture )
            _texture->flush();

        delete _texture;
        delete _eventHandler;
        delete _stream;
    }

    void swapBuffer()
    {
        // wait for completion of previous send
        _running = _sendFuture.get();

        const PixelViewport& pvp = _channel->getPixelViewport();
        const size_t bytesPerPixel = 4;
        const size_t newSize = pvp.w * pvp.h * bytesPerPixel;
        buffer.resize( newSize );

        // OPT: use FBO texture directly to download
        if( !_fboDownload( ))
            _textureDownload();

        const Viewport& vp = _channel->getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::dc::ImageWrapper::swapYAxis( buffer.getData(), pvp.w, pvp.h,
                                       bytesPerPixel );
        ::dc::ImageWrapper imageWrapper( buffer.getData(), pvp.w, pvp.h,
                                         ::dc::RGBA, offsX, offsY );
        imageWrapper.compressionPolicy = ::dc::COMPRESSION_ON;
        imageWrapper.compressionQuality = 100;

        _sendFuture = _stream->asyncSend( imageWrapper );
    }

    ::dc::Stream* _stream;
    EventHandler* _eventHandler;
    eq::Channel* _channel;
    lunchbox::Bufferb buffer;
    ::dc::Stream::Future _sendFuture;
    bool _running;
    util::Texture* _texture;

private:
    bool _fboDownload()
    {
        const SystemWindow* sysWindow = _channel->getWindow()->getSystemWindow();
        const util::FrameBufferObject* fbo = sysWindow->getFrameBufferObject();
        if( !fbo || fbo->getColorTextures().size() != 1 )
            return false;

        const util::Texture* texture = fbo->getColorTextures().front();
        const PixelViewport& pvp = _channel->getPixelViewport();
        if( texture->getWidth() != pvp.w || texture->getHeight() != pvp.h )
            return false;

        texture->download( buffer.getData( ));
        return true;
    }

    void _textureDownload()
    {
        if( !_texture )
            _texture = new util::Texture( GL_TEXTURE_RECTANGLE_ARB,
                            _channel->getObjectManager().glewGetContext( ));

        const PixelViewport& pvp = _channel->getPixelViewport();
        _texture->copyFromFrameBuffer( GL_RGBA, pvp );
        // Needed as copyFromFrameBuffer only grows the texture!
        _texture->resize( pvp.w, pvp.h );
        _texture->download( buffer.getData( ));
    }
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

    if( !_impl->_eventHandler && _impl->_stream->registerForEvents( true ))
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

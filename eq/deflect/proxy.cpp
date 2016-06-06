
/* Copyright (c) 2013-2015, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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
#include "../gl.h"
#include "../image.h"
#include "../pipe.h"
#include "../systemWindow.h"
#include "../view.h"
#include "../window.h"
#include "../windowSystem.h"

#include <eq/fabric/drawableConfig.h>
#include <eq/util/objectManager.h>
#include <eq/util/texture.h>

#include <lunchbox/buffer.h>
#include <deflect/Stream.h>

namespace eq
{
namespace deflect
{

::deflect::Stream::Future make_ready_future( const bool value )
{
    boost::promise< bool > promise;
    promise.set_value( value );
    return promise.get_future();
}

class Proxy::Impl : public boost::noncopyable
{
public:
    explicit Impl( Channel& channel )
        : _channel( channel )
        , _sendFuture( make_ready_future( false ))
        , _running( false )
        , _navigationMode( Proxy::MODE_ROTATE )
    {
        const DrawableConfig& dc = _channel.getDrawableConfig();
        if( dc.colorBits != 8 )
        {
            LBWARN << "Can only stream 8-bit RGB(A) framebuffers to "
                   << "Deflect host, got " << dc.colorBits << " color bits"
                   << std::endl;
            return;
        }

        const std::string& deflectHost =
               _channel.getView()->getSAttribute( View::SATTR_DEFLECT_HOST );
        const std::string& name =
             _channel.getView()->getSAttribute( View::SATTR_DEFLECT_ID );
        _stream.reset( new ::deflect::Stream( name, deflectHost ));
        if( !_stream->isConnected( ))
        {
            LBWARN << "Could not connect to Deflect host: " << deflectHost
                   << std::endl;
            return;
        }

        _running = true;
        _sendFuture = make_ready_future( true );
    }

    ~Impl()
    {
        // wait for completion of previous send
        _sendFuture.wait();
    }

    void notifyNewImage( Channel& channel, const Image& image )
    {
        LBASSERT( &channel == &_channel );

        // wait for completion of previous send
        _running = _sendFuture.get();

        // copy pixels to perform swapYAxis()
        const size_t dataSize = image.getPixelDataSize( Frame::BUFFER_COLOR );
        _buffer.replace( image.getPixelPointer( Frame::BUFFER_COLOR ), dataSize);
        const PixelViewport& pvp = image.getPixelViewport();
        ::deflect::ImageWrapper::swapYAxis( _buffer.getData(), pvp.w, pvp.h,
                                     image.getPixelSize( Frame::BUFFER_COLOR ));

        // determine image offset wrt global view
        const Viewport& vp = channel.getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::deflect::ImageWrapper imageWrapper( _buffer.getData(), pvp.w, pvp.h,
                                              ::deflect::BGRA, offsX, offsY );
        imageWrapper.compressionPolicy = ::deflect::COMPRESSION_ON;
        imageWrapper.compressionQuality = 100;

        _sendFuture = _stream->asyncSend( imageWrapper );
    }

    std::unique_ptr< ::deflect::Stream > _stream;
    std::unique_ptr< EventHandler > _eventHandler;
    Channel& _channel;
    lunchbox::Bufferb _buffer;
    ::deflect::Stream::Future _sendFuture;
    bool _running;
    Proxy::NavigationMode _navigationMode;
};

Proxy::Proxy( Channel& channel )
    : ResultImageListener()
    , _impl( new Impl( channel ))
{
    channel.addResultImageListener( this );
}

Proxy::~Proxy()
{
    _impl->_channel.removeResultImageListener( this );
}

void Proxy::notifyNewImage( Channel& channel, const Image& image )
{
    _impl->notifyNewImage( channel, image );

    if( !_impl->_eventHandler && _impl->_stream->registerForEvents( true ))
    {
        _impl->_eventHandler.reset( new EventHandler( this ));
        LBDEBUG << "Installed event handler for Deflect proxy" << std::endl;
    }
}

Channel& Proxy::getChannel()
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

::deflect::Event Proxy::getEvent() const
{
    return _impl->_stream->getEvent();
}

void Proxy::setNavigationMode( Proxy::NavigationMode mode )
{
    _impl->_navigationMode = mode;
}

Proxy::NavigationMode Proxy::getNavigationMode() const
{
    return _impl->_navigationMode;
}

std::string Proxy::getHelp() const
{
    switch( _impl->_navigationMode )
    {
    case MODE_PAN:
        return "Pan mode";
    case MODE_ROTATE:
    default:
        return "Rotate mode";
    }
}

}
}

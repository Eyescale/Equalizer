
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

#include <dcStream.h>
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
        : _dcSocket( 0 )
        , _eventHandler( 0 )
        , _channel( channel )
        , _buffer( 0 )
        , _size( 0 )
        , _isRunning( false )
        , _interaction( false )
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
        _dcSocket = dcStreamConnect( dcHost.c_str(), false );
        if( !_dcSocket )
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }

        _interaction = dcStreamBindInteraction( _dcSocket,
                                                _channel->getView()->getName());
        if( !_interaction )
        {
            LBWARN << "Could not bind interaction events to DisplayCluster"
                   << std::endl;
        }

        _isRunning = true;
    }

    ~Proxy()
    {
        delete _eventHandler;

        dcStreamDisconnect( _dcSocket );
        delete _buffer;
    }

    void swapBuffer()
    {
        const PixelViewport& pvp = _channel->getPixelViewport();
        const size_t newSize = pvp.w * pvp.h * 4;
        if( !_buffer || _size != newSize )
        {
            delete _buffer;
            _buffer = new unsigned char[newSize];
            _size = newSize;
        }

        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( pvp.x, pvp.y, pvp.w, pvp.h, GL_RGBA, GL_UNSIGNED_BYTE,
                      _buffer );

        const Viewport& vp = _channel->getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        const DcStreamParameters parameters =
                dcStreamGenerateParameters( _channel->getView()->getName(),
                                            offsX, offsY, pvp.w, pvp.h,
                                            width, height );
        dcStreamIncrementFrameIndex();
        _isRunning = dcStreamSend( _dcSocket, _buffer, offsX, offsY, pvp.w, 0,
                                   pvp.h, RGBA, parameters );
    }

    DcSocket* _dcSocket;
    EventHandler* _eventHandler;
    eq::Channel* _channel;
    unsigned char* _buffer;
    size_t _size;
    bool _isRunning;
    bool _interaction;
};
}

Proxy::Proxy( Channel* channel )
    : _impl( new detail::Proxy( channel ))
{
    if( _impl->_interaction )
        _impl->_eventHandler = new EventHandler( this );
}

Proxy::~Proxy()
{
    delete _impl;
}

void Proxy::swapBuffer()
{
    _impl->swapBuffer();
}

Channel* Proxy::getChannel()
{
    return _impl->_channel;
}

int Proxy::getSocketDescriptor() const
{
    return dcSocketDescriptor( _impl->_dcSocket );
}

bool Proxy::hasNewInteractionState()
{
    return dcHasNewInteractionState( _impl->_dcSocket );
}

bool Proxy::isRunning() const
{
    return _impl->_isRunning;
}

void Proxy::stopRunning()
{
    _impl->_isRunning = false;
}

InteractionState Proxy::getInteractionState() const
{
    return dcStreamGetInteractionState( _impl->_dcSocket );
}

}
}

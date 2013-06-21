
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

#include "dcProxy.h"

#include "channel.h"
#include "view.h"
#include <eq/fabric/drawableConfig.h>

#include <dcStream.h>
#include <GL/gl.h>

namespace eq
{
namespace detail
{
class DcProxy
{
public:
    DcProxy( eq::Channel* channel )
        : _channel( channel )
        , _buffer( 0 )
        , _size( 0 )
        , _isRunning( false )
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
        _dcSocket = dcStreamConnect( dcHost.c_str( ));
        if( !_dcSocket )
        {
            LBWARN << "Could not connect to DisplayCluster host: " << dcHost
                   << std::endl;
            return;
        }
        _isRunning = true;
    }

    ~DcProxy()
    {
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
        const int32_t offsY = vp.y * height;

        const DcStreamParameters parameters =
                dcStreamGenerateParameters( _channel->getView()->getName(),
                                            0,  // TODO: get channel index wrt view
                                            offsX, offsY, pvp.w, pvp.h,
                                            width, height );
        dcStreamIncrementFrameIndex();
        _isRunning = dcStreamSend( _dcSocket, _buffer, offsX, offsY, pvp.w, 0,
                                   pvp.h, RGBA, parameters );
    }

    DcSocket* _dcSocket;
    eq::Channel* _channel;
    unsigned char* _buffer;
    size_t _size;
    bool _isRunning;
};
}

DcProxy::DcProxy( Channel* channel )
    : _impl( new detail::DcProxy( channel ))
{
}

DcProxy::~DcProxy()
{
    delete _impl;
}

void DcProxy::swapBuffer()
{
    _impl->swapBuffer();
}

bool DcProxy::isRunning() const
{
    return _impl->_isRunning;
}

}

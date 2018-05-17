
/* Copyright (c) 2013-2017, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <deflect/Stream.h>
#include <lunchbox/buffer.h>

namespace eq
{
namespace deflect
{
class Proxy::Impl : public boost::noncopyable
{
public:
    explicit Impl(Channel& ch)
        : channel(ch)
    {
        const DrawableConfig& dc = channel.getDrawableConfig();
        if (dc.colorBits != 8)
        {
            LBWARN << "Can only stream 8-bit RGB(A) framebuffers to "
                   << "Deflect host, got " << dc.colorBits << " color bits"
                   << std::endl;
            return;
        }

        const std::string& deflectHost =
            channel.getView()->getSAttribute(View::SATTR_DEFLECT_HOST);
        const std::string& name =
            channel.getView()->getSAttribute(View::SATTR_DEFLECT_ID);
        stream.reset(new ::deflect::Stream(name, deflectHost));
        if (!stream->isConnected())
        {
            LBWARN << "Could not connect to Deflect host: " << deflectHost
                   << std::endl;
            stream.reset();
        }
    }

    ~Impl()
    {
        for (size_t i = 0; i < NUM_EYES; ++i)
            if (_sendFuture[i].valid())
                _sendFuture[i].wait();
        if (_finishFuture.valid())
            _finishFuture.wait();
    }

    void notifyNewImage(Channel&, const Image& image)
    {
        switch (channel.getEye())
        {
        case eq::EYE_LEFT:
            _send(::deflect::View::left_eye, EYE_LEFT_BIT, image);
            return;

        case eq::EYE_RIGHT:
            _send(::deflect::View::right_eye, EYE_RIGHT_BIT, image);
            return;

        default:
            _send(::deflect::View::mono, EYE_CYCLOP_BIT, image);
            return;
        }
    }

    void finishFrame()
    {
        if (_finishFuture.valid() && !_finishFuture.get())
            stream.reset();
        if (!stream)
            return;

        for (size_t i = 0; i < NUM_EYES; ++i)
        {
            if (_sendFuture[i].valid())
            {
                _finishFuture = stream->finishFrame();
                return;
            }
        }
    }

    Channel& channel;
    std::unique_ptr<::deflect::Stream> stream;
    std::unique_ptr<EventHandler> eventHandler;

private:
    void _send(const ::deflect::View view, const Eye eye, const Image& image)
    {
        if (_sendFuture[eye].valid() && !_sendFuture[eye].get())
            stream.reset();
        if (!stream)
            return;

        // copy pixels to retain data until _sendFuture is ready
        _buffer[eye].replace(image.getPixelPointer(Frame::Buffer::color),
                             image.getPixelDataSize(Frame::Buffer::color));

        // determine image offset wrt global view
        const PixelViewport& pvp = image.getPixelViewport();
        const Viewport& vp = channel.getViewport();
        const int32_t width = pvp.w / vp.w;
        const int32_t height = pvp.h / vp.h;
        const int32_t offsX = vp.x * width;
        const int32_t offsY = height - (vp.y * height + vp.h * height);

        ::deflect::ImageWrapper imageWrapper(_buffer[eye].getData(), pvp.w,
                                             pvp.h, ::deflect::BGRA, offsX,
                                             offsY);
        imageWrapper.compressionPolicy = ::deflect::COMPRESSION_ON;
        imageWrapper.compressionQuality = 100;
        imageWrapper.view = view;
        imageWrapper.rowOrder = ::deflect::RowOrder::bottom_up;

        _sendFuture[eye] = stream->send(imageWrapper);
    }

    lunchbox::Bufferb _buffer[NUM_EYES];
    ::deflect::Stream::Future _sendFuture[NUM_EYES];
    ::deflect::Stream::Future _finishFuture;
};

Proxy::Proxy(Channel& channel)
    : ResultImageListener()
    , _impl(new Impl(channel))
{
    channel.addResultImageListener(this);
}

Proxy::~Proxy()
{
    _impl->channel.removeResultImageListener(this);
}

void Proxy::notifyNewImage(Channel& channel, const Image& image)
{
    _impl->notifyNewImage(channel, image);

    if (!_impl->eventHandler && _impl->stream->registerForEvents(true))
    {
        _impl->eventHandler.reset(new EventHandler(this));
        LBDEBUG << "Installed event handler for Deflect proxy" << std::endl;
    }
}

void Proxy::notifyFinishFrame()
{
    _impl->finishFrame();
}

Channel& Proxy::getChannel()
{
    return _impl->channel;
}

int Proxy::getSocketDescriptor() const
{
    return _impl->stream->getDescriptor();
}

bool Proxy::hasNewEvent() const
{
    return _impl->stream->hasEvent();
}

bool Proxy::isRunning() const
{
    return _impl->stream != nullptr;
}

void Proxy::stopRunning()
{
    _impl->stream.reset();
}

::deflect::Event Proxy::getEvent() const
{
    return _impl->stream->getEvent();
}
}
}

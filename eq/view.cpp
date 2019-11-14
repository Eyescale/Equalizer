
/* Copyright (c) 2008-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "view.h"

#include "compositor.h"
#include "config.h"
#include "eventICommand.h"
#include "image.h"
#include "imageOp.h"
#include "layout.h"
#include "observer.h"
#include "pipe.h"
#include "server.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/sizeEvent.h>

#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/spinLock.h>

namespace eq
{
namespace detail
{
class View
{
public:
    lunchbox::SpinLock eventLock; //!< event-handling resize synchronizer

    /** Unmodified, baseline view frustum data, used for resizing. */
    Frustum baseFrustum;

    struct Screenshot
    {
        std::unique_ptr<eq::Image> _image;
        Viewport _viewport;

        bool isComplete() const { return _viewport == Viewport::FULL; }
        void composite(const Viewport& viewport, const eq::Image& image)
        {
            if (!_image)
            {
                _viewport = viewport;
                _image.reset(new eq::Image(image));
                return;
            }

            ImageOps ops;
            ImageOp current;
            current.image = _image.get();
            ImageOp newImage;
            newImage.image = &image;

            ops.push_back(current);
            ops.push_back(newImage);
            _image.reset(
                new eq::Image(*Compositor::mergeImagesCPU(ops, false)));
            _viewport.unite(viewport);
        }
    };

    std::map<uint32_t, Screenshot> screenshot;
    eq::View::ScreenshotFunc screenshotFunc;
};
} // namespace detail

typedef fabric::View<Layout, View, Observer> Super;

View::View(Layout* parent)
    : Super(parent)
    , _impl(new detail::View)
    , _pipe(0)
{
}

View::~View()
{
    delete _impl;
}

void View::deserialize(co::DataIStream& is, const uint64_t dirtyBits)
{
    Super::deserialize(is, dirtyBits);
    if (_impl->baseFrustum.getCurrentType() == TYPE_NONE &&
        (dirtyBits & DIRTY_FRUSTUM))
    {
        _impl->baseFrustum = *this; // save baseline data for resizing
    }
}

void View::detach()
{
    // if pipe is not running, detach comes from _flushViews in state stopping
    //  Don't send command to stopping pipe (see issue #11)
    if (_pipe && _pipe->isRunning())
    {
        // local command dispatching
        co::ObjectOCommand(_pipe, getLocalNode(), fabric::CMD_PIPE_DETACH_VIEW,
                           co::COMMANDTYPE_OBJECT, _pipe->getID(),
                           CO_INSTANCE_ALL)
            << getID();
    }
    Super::detach();
}

Config* View::getConfig()
{
    Layout* layout = getLayout();
    if (layout)
        return layout->getConfig();

    if (_pipe)
        return _pipe->getConfig();

    LBUNREACHABLE;
    return 0;
}

const Config* View::getConfig() const
{
    const Layout* layout = getLayout();
    if (layout)
        return layout->getConfig();

    if (_pipe)
        return _pipe->getConfig();

    LBUNREACHABLE;
    return 0;
}

ServerPtr View::getServer()
{
    Config* config = getConfig();
    LBASSERT(config);
    return (config ? config->getServer() : 0);
}

const Frustum& View::getBaseFrustum() const
{
    return _impl->baseFrustum;
}

bool View::handleEvent(const EventType type, const SizeEvent& event)
{
    if (type != EVENT_VIEW_RESIZE)
        return false;

    if (event.dw == 0.f || event.dh == 0.f)
        return true;

    switch (getCurrentType())
    {
    case TYPE_WALL:
    {
        const float ratio(event.dw / event.dh);
        Wall wall(_impl->baseFrustum.getWall());
        wall.resizeHorizontal(ratio);

        lunchbox::ScopedFastWrite mutex(_impl->eventLock);
        setWall(wall);
        return true;
    }

    case View::TYPE_PROJECTION:
    {
        const float ratio(event.dw / event.dh);
        eq::Projection projection(_impl->baseFrustum.getProjection());
        projection.resizeHorizontal(ratio);

        lunchbox::ScopedFastWrite mutex(_impl->eventLock);
        setProjection(projection);
        return true;
    }

    default:
        LBUNIMPLEMENTED;
    /* fall-thru */
    case eq::View::TYPE_NONE:
        return false;
    }
}

void View::enableScreenshot(Frame::Buffer buffers, const ScreenshotFunc& func)
{
    Super::_setScreenshotBuffers(buffers);
    _impl->screenshotFunc = func;
}

void View::disableScreenshot()
{
    Super::_setScreenshotBuffers(Frame::Buffer::none);
}

void View::sendScreenshotEvent(const Viewport& viewport,
                               const uint32_t frameNumber, const Image& image)
{
    getConfig()->sendEvent(EVENT_VIEW_SCREENSHOT)
        << getID() << viewport << frameNumber << image;
}

bool View::handleEvent(EventICommand& command)
{
    switch (command.getEventType())
    {
    case EVENT_VIEW_SCREENSHOT:
        return _handleScreenshot(command);
    }
    return false;
}

bool View::_handleScreenshot(EventICommand& command)
{
    const auto& vp = command.read<Viewport>();
    const auto frameNumber = command.read<uint32_t>();
    const auto& newImage = command.read<Image>();

    auto& screenshot = _impl->screenshot[frameNumber];
    screenshot.composite(vp, newImage);

    if (screenshot.isComplete())
    {
        _impl->screenshotFunc(frameNumber, *screenshot._image);
        _impl->screenshot.erase(frameNumber);
    }

    return true;
}
} // namespace eq

#include <eq/fabric/view.ipp>
template class eq::fabric::View<eq::Layout, eq::View, eq::Observer>;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator<<(std::ostream&,
                                                           const eq::Super&);
/** @endcond */

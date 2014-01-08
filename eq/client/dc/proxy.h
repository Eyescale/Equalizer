
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

#ifndef EQ_DC_PROXY_H
#define EQ_DC_PROXY_H

#include <eq/client/types.h>
#include <dc/types.h>

namespace eq
{
namespace dc
{
namespace detail { class Proxy; }

/** @internal */
class Proxy
{
public:
    /** Construct a DisplayCluster proxy associated to a destination channel. */
    Proxy( Channel* channel );

    /** Destruct the DisplayCluster proxy. */
    ~Proxy();

    /** Stream the pixel data of the currently bound buffer to DisplayCluster.
     *
     * Has to be called from Channel::frameViewFinish.
     */
    void swapBuffer();

    /** @return the associated destination channel. */
    Channel* getChannel();

    /** @return the underlying socket descriptor. */
    int getSocketDescriptor() const;

    /** @return true if a new Event was sent by DisplayCluster. */
    bool hasNewEvent();

    /** @return whether the application is running in DisplayCluster. */
    bool isRunning() const;

    /**
     * Signal the proxy to stop running.
     *
     * This is called if the application is closed in DisplayCluster.
     */
    void stopRunning();

    /** @return the latest window Event.
     *  @sa hasNewEvent()
     */
    ::dc::Event getEvent() const;

private:
    detail::Proxy* const _impl;
};
}
}
#endif // EQ_DC_PROXY_H

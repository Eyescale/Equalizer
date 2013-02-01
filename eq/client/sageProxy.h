
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

#ifndef EQ_SAGEPROXY_H
#define EQ_SAGEPROXY_H

#include <eq/client/types.h>

class sail;

namespace eq
{
namespace detail { class SageProxy; }

/**
 * @internal
 * A SageProxy is associated to one destination channel to stream data to a
 * SAGE-driven display. Several SageProxy instances appear as one SAGE
 * application when they share the same Sage appID. Additionally to the pixel
 * data streaming, the SageProxy installs an event handler for incoming messages
 * from SAGE on that application.
 */
class SageProxy
{
public:
    /** Construct a new SAGE proxy associated to a destination channel. */
    SageProxy( Channel* channel );

    /** Destruct the SAGE proxy and close the SAGE application. */
    ~SageProxy();

    /**
     * Stream the pixel data of the currently bound buffer to SAGE.
     *
     * Has to be called from Channel::frameViewFinish.
     */
    void swapBuffer();

    /** @return the associated destination channel. */
    Channel* getChannel();

    /** @return the SAGE sail instance. */
    sail* getSail();

    /**
     * Signal the proxy to stop running.
     *
     * This is called if the application is closed in SAGE.
     */
    void stopRunning();

    /** @return whether the application is running in SAGE. */
    bool isRunning() const;

private:
    detail::SageProxy* const _impl;
};
}

#endif // EQ_SAGEPROXY_H

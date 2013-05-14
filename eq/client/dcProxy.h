
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

#ifndef EQ_DCPROXY_H
#define EQ_DCPROXY_H

#include <eq/client/types.h>

class sail;

namespace eq
{
namespace detail { class DcProxy; }

/** @internal */
class DcProxy
{
public:
    /** Construct a DisplayCluster proxy associated to a destination channel. */
    DcProxy( Channel* channel );

    /** Destruct the DisplayCluster proxy. */
    ~DcProxy();

    /** Stream the pixel data of the currently bound buffer to DisplayCluster.
     *
     * Has to be called from Channel::frameViewFinish.
     */
    void swapBuffer();

    /** @return whether the application is running in DisplayCluster. */
    bool isRunning() const;

private:
    detail::DcProxy* const _impl;
};
}

#endif // EQ_DCPROXY_H

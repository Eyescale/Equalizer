
/* Copyright (c) 2009-2015, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "channel.h"
#include <eq/x11/window.h>
#include <eq/system.h>
#include <eq/window.h>

namespace eqCpu
{

Channel::Channel( eq::Window* parent )
    : eq::Channel( parent )
{
}

void Channel::frameDraw( const eq::uint128_t& )
{
    eq::x11::Window* x11Window = static_cast< eq::x11::Window* >(
                               getWindow()->getSystemWindow( ));
    Display* display = x11Window->getXDisplay();
    XID drawable = x11Window->getXDrawable();
    const int screen = DefaultScreen( display );

    XFillRectangle( display, drawable, DefaultGC( display, screen ),
                    20, 20, 50, 50 );
}

void Channel::frameClear( const eq::uint128_t& )
{
    const eq::PixelViewport& pvp = getPixelViewport();
    eq::x11::Window* x11Window = static_cast< eq::x11::Window* >(
                               getWindow()->getSystemWindow( ));
    Display* display = x11Window->getXDisplay();
    XID drawable = x11Window->getXDrawable();

    XClearArea( display, drawable, pvp.x, pvp.y, pvp.w, pvp.h, false );
}

void Channel::frameReadback( const eq::uint128_t&, const eq::Frames& )
{
    LBUNIMPLEMENTED;
}

void Channel::frameAssemble( const eq::uint128_t&, const eq::Frames& )
{
    LBUNIMPLEMENTED;
}

void Channel::setupAssemblyState()
{
}

void Channel::resetAssemblyState()
{
}

}

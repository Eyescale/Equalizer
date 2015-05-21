
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

#include "window.h"
#include <eq/glx/pipe.h>
#include <eq/x11/window.h>
#include <eq/pipe.h>

namespace eqCpu
{

Window::Window( eq::Pipe* parent )
    : eq::Window( parent )
{
}

bool Window::configInitSystemWindow( const eq::uint128_t& initID )
{
    eq::Pipe* pipe = getPipe();
    eq::glx::Pipe* glxPipe = dynamic_cast< eq::glx::Pipe* >(
        pipe->getSystemPipe( ));

    if( !glxPipe )
        return eq::Window::configInitSystemWindow( initID );

    eq::x11::Window* systemWindow =
        new eq::x11::Window( *this, getSettings(), glxPipe->getXDisplay( ));

    if( !systemWindow->configInit( ))
    {
        delete systemWindow;
        return false;
    }

    setSystemWindow( systemWindow );
    return true;
}

}

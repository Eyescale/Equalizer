
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
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

#include "windowFactory.h"

#include "window.h"
#include "windowImpl.h"

namespace eq
{
namespace qt
{

detail::Window* WindowFactory::onCreateImpl( const WindowSettings& settings,
                                             QThread* thread_ )
{
    return Window::createImpl( settings, thread_ );
}

void WindowFactory::onDestroyImpl( detail::Window* window )
{
    delete window;
}

}
}

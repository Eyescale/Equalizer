
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

#include "eq/pipe.h"

#include <QGuiApplication>

namespace eq
{
namespace qt
{

detail::Window* WindowFactory::onCreateImpl( const eq::Pipe* pipe,
                                             const WindowSettings& settings,
                                             QThread* thread_ )
{
    // Trying to infer the screen to use from the pipe device number.
    // We will simply assume that each QScreen belongs to a display and that
    // each GPU drives a different display. This will work for most setups,
    // but will break at the moment a single GPU is driving more than one
    // screen. The only solution in those cases is to adjust manually the
    // configuration to the setup.
    QGuiApplication* app =
        dynamic_cast< QGuiApplication* >( QCoreApplication::instance( ));
    LBASSERT(app);

    QList< QScreen* > screens = app->screens();
    QScreen* screen;
    const unsigned int device = pipe->getDevice();
    if( device == LB_UNDEFINED_UINT32 )
        screen = app->primaryScreen();
    else if( int(device) >= screens.size( ))
    {
        LBWARN << "Cannot used device number " << device << ", Qt detected "
                  "only " << screens.size() << " screens. Using the default"
                  " screen instead" << std::endl;
        screen = app->primaryScreen();
    }
    else
    {
        screen = screens[device];
    }
    return Window::createImpl( settings, screen, thread_ );
}

void WindowFactory::onDestroyImpl( detail::Window* window )
{
    delete window;
}

}
}

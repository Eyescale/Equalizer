
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "windowSettings.h"


namespace eq
{
namespace detail
{
class WindowSettings
{
public:
    WindowSettings()
        : sharedContextWindow( 0 )
    {
    }
    ~WindowSettings() {}

    WindowSettings( const WindowSettings& rhs )
        : sharedContextWindow( rhs.sharedContextWindow )
    {
    }

    WindowSettings& operator=( const WindowSettings& rhs )
    {
        if( this == &rhs )
            return *this;

        sharedContextWindow = rhs.sharedContextWindow;
        return *this;
    }

    const SystemWindow* sharedContextWindow;
};
}

WindowSettings::WindowSettings()
    : fabric::WindowSettings()
    , _impl( new detail::WindowSettings( ))
{
}

WindowSettings::WindowSettings( const WindowSettings& rhs )
    : fabric::WindowSettings( rhs )
    , _impl( new detail::WindowSettings( *rhs._impl ))
{
}

WindowSettings::~WindowSettings()
{
    delete _impl;
}

WindowSettings& WindowSettings::operator=( const WindowSettings& rhs )
{
    fabric::WindowSettings::operator =( rhs );
    *_impl = *rhs._impl;
    return *this;
}

void WindowSettings::setSharedContextWindow( const SystemWindow* window )
{
    _impl->sharedContextWindow = window;
}

const SystemWindow* WindowSettings::getSharedContextWindow() const
{
    return _impl->sharedContextWindow;
}

}

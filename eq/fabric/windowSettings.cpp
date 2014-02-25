
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
#include "window.h"
#include "iAttribute.h"

#include <co/dataOStream.h>
#include <co/dataIStream.h>
#include <GL/gl.h>

namespace eq
{
namespace fabric
{
namespace detail
{
class WindowSettings
{
public:
    WindowSettings()
    {
        memset( iAttributes, 0xff,
                fabric::WindowSettings::IATTR_ALL * sizeof( int32_t ));
    }
    ~WindowSettings() {}

    WindowSettings( const WindowSettings& rhs )
        : pixelViewport( rhs.pixelViewport )
        , windowName( rhs.windowName )
    {
        memcpy( iAttributes, rhs.iAttributes,
                fabric::WindowSettings::IATTR_ALL * sizeof( int32_t ));
    }

    WindowSettings& operator=( const WindowSettings& rhs )
    {
        if( this == &rhs )
            return *this;

        memcpy( iAttributes, rhs.iAttributes,
                fabric::WindowSettings::IATTR_ALL * sizeof( int32_t ));
        pixelViewport = rhs.pixelViewport;
        windowName = rhs.windowName;
        return *this;
    }

    int32_t iAttributes[ fabric::WindowSettings::IATTR_ALL ];

    PixelViewport pixelViewport;
    std::string windowName;
};
}

WindowSettings::WindowSettings()
    : _impl( new detail::WindowSettings( ))
{
}

WindowSettings::WindowSettings( const  WindowSettings& rhs )
    : _impl( new detail::WindowSettings( *rhs._impl ))
{
}

WindowSettings::~WindowSettings()
{
    delete _impl;
}

WindowSettings& WindowSettings::operator=( const WindowSettings& rhs )
{
    *_impl = *rhs._impl;
    return *this;
}

bool WindowSettings::setIAttribute( const IAttribute attr, const int32_t value )
{
    if( _impl->iAttributes[attr] == value )
        return false;

    _impl->iAttributes[attr] = value;
    return true;
}

int32_t WindowSettings::getIAttribute( const IAttribute attr ) const
{
    return _impl->iAttributes[attr];
}

const PixelViewport& WindowSettings::getPixelViewport() const
{
    return _impl->pixelViewport;
}

void WindowSettings::setPixelViewport( const PixelViewport& pvp )
{
    _impl->pixelViewport = pvp;
}

void WindowSettings::setName( const std::string& name )
{
    _impl->windowName = name;
}

const std::string& WindowSettings::getName() const
{
    return _impl->windowName;
}

uint32_t WindowSettings::getColorFormat() const
{
    switch( getIAttribute( IATTR_PLANES_COLOR ))
    {
        case RGBA32F:  return GL_RGBA32F;
        case RGBA16F:  return GL_RGBA16F;
        default:       return GL_RGBA;
    }
}

void WindowSettings::serialize( co::DataOStream& os ) const
{
    os << co::Array< int32_t >( _impl->iAttributes, IATTR_ALL )
       << _impl->windowName;
}

void WindowSettings::deserialize( co::DataIStream& is )
{
    is >> co::Array< int32_t >( _impl->iAttributes, IATTR_ALL )
       >> _impl->windowName;
}

}
}

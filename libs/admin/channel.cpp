
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "channel.h"

#include "server.h"
#include "window.h"

namespace eq
{
namespace admin
{
typedef fabric::Channel< Window, Channel > Super;

Channel::Channel( Window* parent )
        : Super( parent )
{}

Channel::~Channel()
{}

Pipe* Channel::getPipe()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getPipe() : 0 );
}
const Pipe* Channel::getPipe() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getPipe() : 0 );
}

Config* Channel::getConfig()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getConfig() : 0 );
}

const Config* Channel::getConfig() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getConfig() : 0 );
}

ServerPtr Channel::getServer()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getServer() : 0 );
}

}
}

#include "../fabric/channel.ipp"
template class eq::fabric::Channel< eq::admin::Window, eq::admin::Channel >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                      const eq::admin::Super& );
/** @endcond */

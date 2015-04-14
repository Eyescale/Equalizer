
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "init.h"

#include <eq/fabric/init.h>
#include <lunchbox/debug.h>

namespace eq
{
namespace server
{
namespace
{
static bool _initialized = false;
}

bool init( const int argc, char** argv )
{
    if( _initialized )
    {
        LBERROR << "Equalizer server library already initialized" << std::endl;
        return false;
    }
    _initialized = true;

    return fabric::init( argc, argv );
}

bool exit()
{
    if( !_initialized )
    {
        LBERROR << "Equalizer server library not initialized" << std::endl;
        return false;
    }
    _initialized = false;

    return fabric::exit();
}

}
}

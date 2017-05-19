
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <co/init.h>
#include <lunchbox/atomic.h>

namespace eq
{
namespace fabric
{
namespace
{
static lunchbox::a_int32_t _initialized;
}
extern void _initErrors();
extern void _exitErrors();

bool init(const int argc, char** argv)
{
    if (++_initialized > 1) // not first
        return true;

    _initErrors();
    return co::init(argc, argv);
}

bool exit()
{
    if (--_initialized > 0) // not last
        return true;

    const bool ret = co::exit();

    _exitErrors();
    return ret;
}
}
}

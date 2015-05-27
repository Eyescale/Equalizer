
/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010-2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "windowStatistics.h"

#include "config.h"
#include "global.h"
#include "pipe.h"
#include "window.h"

#include <cstdio>

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

WindowStatistics::WindowStatistics( const Statistic::Type type,
                                    Window* window )
        : StatisticSampler< Window >( type, window )
{
    const int32_t hint = _owner->getIAttribute( WindowSettings::IATTR_HINT_STATISTICS );
    if( hint == OFF )
        return;

    const std::string& name = window->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "Window %s",
                  window->getID().getShortString().c_str( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str());
    event.data.statistic.resourceName[31] = 0;

    if( type != Statistic::WINDOW_FPS && hint == NICEST )
        window->finish();

    event.data.statistic.startTime  = window->getConfig()->getTime();
}


WindowStatistics::~WindowStatistics()
{
    const int32_t hint = _owner->getIAttribute( WindowSettings::IATTR_HINT_STATISTICS );
    if( hint == OFF )
        return;

    if( event.data.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    if( event.data.statistic.type != Statistic::WINDOW_FPS && hint == NICEST )
        _owner->finish();

    event.data.statistic.endTime = _owner->getConfig()->getTime();
    if( event.data.statistic.endTime <= event.data.statistic.startTime )
        event.data.statistic.endTime = event.data.statistic.startTime + 1;
    _owner->processEvent( event.data );
}

}

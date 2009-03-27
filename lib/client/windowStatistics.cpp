
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

WindowStatistics::WindowStatistics( const Statistic::Type type, 
                                    Window* window )
        : _window( window )
{
    const int32_t hint = _window->getIAttribute( Window::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    _event.type                  = Event::STATISTIC;
    _event.originator            = window->getID();
    _event.statistic.type        = type;
    _event.statistic.frameNumber = window->getPipe()->getCurrentFrame();

    const std::string& name = window->getName();
    if( name.empty( ))
        snprintf( _event.statistic.resourceName, 32, "window %d",
                  window->getID( ));
    else
        snprintf( _event.statistic.resourceName, 32, "%s", name.c_str( ));

    if( hint == NICEST )
        window->finish();
    _event.statistic.startTime  = window->getConfig()->getTime();
}


WindowStatistics::~WindowStatistics()
{
    const int32_t hint = _window->getIAttribute( Window::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( _event.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    if( hint == NICEST )
        _window->finish();

    _event.statistic.endTime = _window->getConfig()->getTime();
    _window->processEvent( _event );
}

}

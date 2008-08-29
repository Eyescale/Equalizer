
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "windowStatistics.h"

#include "window.h"
#include "global.h"

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

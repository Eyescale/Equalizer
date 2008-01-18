
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"
#include "configEvent.h"

using namespace std;

namespace eqPixelBench
{
Config::Config()
        : _clock(0)
{
}

Config::~Config()
{
    delete _clock;
    _clock = 0;
}

uint32_t Config::startFrame( const uint32_t frameID )
{
    if( !_clock )
        _clock = new eqBase::Clock;

    _clock->reset();
    return eq::Config::startFrame( frameID );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case ConfigEvent::READBACK:
        case ConfigEvent::ASSEMBLE:
        case ConfigEvent::START_LATENCY:
            cout << static_cast< const ConfigEvent* >( event ) << endl;
            return true;

        default:
            return eq::Config::handleEvent( event );
    }
}
}

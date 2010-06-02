
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "config.h"
#include "configEvent.h"

using namespace std;

namespace eqTransfertDSO
{
Config::Config( eq::base::RefPtr< eq::Server > parent )
        : eq::Config( parent )
        , _clock(0)
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
        _clock = new eq::base::Clock;

    _clock->reset();
    return eq::Config::startFrame( frameID );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case ConfigEvent::READBACK:
        case ConfigEvent::READBACK_PBO:
        case ConfigEvent::ASSEMBLE:
        case ConfigEvent::START_LATENCY:
            cout << static_cast< const ConfigEvent* >( event ) << endl;
            return true;

        default:
            return eq::Config::handleEvent( event );
    }
}
}


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

#include "configEvent.h"

using namespace std;

namespace eqTransfertDSO
{
std::ostream& operator << ( std::ostream& os, const ConfigEvent* event )
{
    switch( event->data.type )
    {
        case ConfigEvent::READBACK:
            os  << "readback";
            break;

        case ConfigEvent::READBACK_PBO:
            os  << "read PBO";
            break;

        case ConfigEvent::ASSEMBLE:
            os  << "assemble";
            break;

        case ConfigEvent::START_LATENCY:
            os  << "        ";
            break;

        default:
            os << static_cast< const eq::ConfigEvent* >( event );
            return os;
    }

    os << " \"" << event->data.user.data << "\" " << event->formatType
       << string( 50-strlen( event->formatType ), ' ' ) << event->area << ": ";

    if( event->msec < 0.0f )
        os << "error 0x" << hex << static_cast< int >( -event->msec ) << dec;
    else
        os << static_cast< uint32_t >( event->area.x() * event->area.y() / 
                                       event->msec  / 1048.576f )
           << "MPix/sec (" << event->msec << "ms, " << 1000.0f / event->msec
           << "FPS)";
    return os;
}
}

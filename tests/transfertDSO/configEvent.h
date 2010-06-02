
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

#ifndef EQ_PIXELBENCH_CONFIGEVENT_H
#define EQ_PIXELBENCH_CONFIGEVENT_H

#include <eq/eq.h>

namespace eqTransfertDSO
{
struct ConfigEvent : public eq::ConfigEvent
{
public:
    enum Type
    {
        READBACK = eq::Event::USER,
        READBACK_PBO,
        ASSEMBLE,
        START_LATENCY
    };

    ConfigEvent()
        {
            size = sizeof( ConfigEvent );
        }

    // channel name is in user event data
    char           formatType[64];
    eq::Vector2i   area;
    float          msec;
};

std::ostream& operator << ( std::ostream& os, const ConfigEvent* event );
}

#endif // EQ_PIXELBENCH_CONFIGEVENT_H


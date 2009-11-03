
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

#ifndef EQ_PLY_CONFIGEVENT_H
#define EQ_PLY_CONFIGEVENT_H

#include <eq/eq.h>

namespace eqPly
{

struct ConfigEvent : public eq::ConfigEvent
{
public:
    enum Type
    {
        IDLE_AA = eq::Event::USER
    };

    ConfigEvent()
    {
        size = sizeof( ConfigEvent );
    }

    uint32_t jitter;
};

std::ostream& operator << ( std::ostream& os, const ConfigEvent* event );
}

#endif // EQ_PLY_CONFIGEVENT_H


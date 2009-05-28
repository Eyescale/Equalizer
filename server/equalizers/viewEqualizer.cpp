
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "viewEqualizer.h"

#include "../compound.h"

namespace eq
{
namespace server
{

ViewEqualizer::ViewEqualizer()
{
    EQINFO << "New view equalizer @" << (void*)this << std::endl;
}

ViewEqualizer::ViewEqualizer( const ViewEqualizer& from )
        : Equalizer( from )
{}

ViewEqualizer::~ViewEqualizer()
{
    attach( 0 );
    EQINFO << "Delete view equalizer @" << (void*)this << std::endl;
}

void ViewEqualizer::attach( Compound* compound )
{
    Equalizer::attach( compound );
}

void ViewEqualizer::notifyUpdatePre( Compound* compound, 
                                     const uint32_t frameNumber )
{
}

std::ostream& operator << ( std::ostream& os, const ViewEqualizer* equalizer)
{
    if( equalizer )
        os << "view_equalizer {}" << std::endl;
    return os;
}

}
}


/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "equalizer.h"

#include "../compound.h"
#include "../config.h"
#include "../log.h"

#include <eq/client.h>
#include <eq/server.h>
#include <co/base/debug.h>

namespace eq
{
namespace server
{

Equalizer::Equalizer()
        : _compound( 0 )
        , _frozen( false )
{
    EQINFO << "New Equalizer @" << (void*)this << std::endl;
}

Equalizer::Equalizer( const Equalizer& from )
        : CompoundListener( from )
        , _compound( 0 )
        , _frozen( from._frozen )
{}

Equalizer::~Equalizer()
{
    attach( 0 );
}

void Equalizer::attach( Compound* compound )
{
    if( _compound )
    {
        _compound->removeListener( this );
        _compound = 0;
    }

    if( compound )
    {
        _compound = compound;
        compound->addListener( this );
    }
}

const Config* Equalizer::getConfig() const
{
    EQASSERT( _compound );
    return _compound->getConfig();
}

}
}

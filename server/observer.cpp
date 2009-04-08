
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "observer.h"

#include "config.h"
#include "paths.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{
namespace server
{

Observer::Observer()
        : _config( 0 )
{}

Observer::Observer( const Observer& from, Config* config )
        : eq::Observer( from )
        , _config( 0 )
{
    config->addObserver( this );
    EQASSERT( _config );
}

Observer::~Observer()
{
}

//void Observer::serialize( net::DataOStream& os, const uint64_t dirtyBits )

ObserverPath Observer::getPath() const
{
    EQASSERT( _config );
    
    const ObserverVector&  observers = _config->getObservers();
    ObserverVector::const_iterator i = std::find( observers.begin(), 
                                                  observers.end(), this );
    EQASSERT( i != observers.end( ));

    ObserverPath path;
    path.observerIndex = std::distance( observers.begin(), i );
    return path;
}

void Observer::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    EQASSERT( getID() != EQ_ID_INVALID );

    session->unmapObject( this );
}

}
}

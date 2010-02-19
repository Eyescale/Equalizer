
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <pthread.h> // needed for mtQueue template instantiation
#include "view.h"
#include "config.h"

namespace eqPly
{

View::View()
        : eq::View()
        , _modelID( EQ_ID_INVALID )
        , _idleSteps( 0 )
{}

View::~View()
{
    _modelID = EQ_ID_INVALID;
    _idleSteps = 0;
}

void View::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::View::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_MODEL )
        os << _modelID;
    if( dirtyBits & DIRTY_IDLE )
        os << _idleSteps;
}

void View::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
{
    eq::View::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_MODEL )
        is >> _modelID;
    if( dirtyBits & DIRTY_IDLE )
        is >> _idleSteps;
}

void View::notifyNewVersion()
{
    EQASSERT( isMaster( ));
    eq::View::notifyNewVersion();

    Config* config = static_cast< Config* >( getSession( ));
    config->notifyNewVersion( this );
}

void View::setModelID( const uint32_t id )
{
    _modelID = id;
    setDirty( DIRTY_MODEL );
}

void View::setIdleSteps( const uint32_t steps )
{
    _idleSteps = steps;
    setDirty( DIRTY_IDLE );
}

}

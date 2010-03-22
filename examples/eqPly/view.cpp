
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

View::View( eq::Layout* parent )
        : eq::View( parent )
        , _proxy( this )
        , _modelID( EQ_ID_INVALID )
        , _idleSteps( 0 )
{
    setUserData( &_proxy );
}

View::~View()
{
    if( _proxy.getID() != EQ_ID_INVALID )
    {
        eq::net::Session* session = _proxy.getSession();
        EQASSERT( session );
        if( _proxy.isMaster( ))
            session->deregisterObject( &_proxy );
        else
            session->unmapObject( &_proxy );
    }

    _modelID = EQ_ID_INVALID;
    _idleSteps = 0;
}

void View::Proxy::serialize( eq::net::DataOStream& os, const
                               uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_MODEL )
        os << _view->_modelID;
    if( dirtyBits & DIRTY_IDLE )
        os << _view->_idleSteps;
}

void View::Proxy::deserialize( eq::net::DataIStream& is,
                               const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_MODEL )
        is >> _view->_modelID;
    if( dirtyBits & DIRTY_IDLE )
    {
        is >> _view->_idleSteps;
        if( isMaster( ))
            setDirty( DIRTY_IDLE ); // redistribute slave settings
    }
}

void View::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
{
    eq::View::deserialize( is, dirtyBits );
    if( _proxy.getID() == EQ_ID_INVALID && getLayout( )) // app view instance
    {
        getSession()->registerObject( &_proxy );
        _proxy.setAutoObsolete( getConfig()->getLatency( ));
    }
}

void View::setModelID( const uint32_t id )
{
    _modelID = id;
    _proxy.setDirty( Proxy::DIRTY_MODEL );
}

void View::setIdleSteps( const uint32_t steps )
{
    _idleSteps = steps;
    _proxy.setDirty( Proxy::DIRTY_IDLE );
}

}

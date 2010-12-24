
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <pthread.h> // needed for mtQueue template instantiation
#include "view.h"
#include "config.h"

namespace eqPly
{

#pragma warning( push )
#pragma warning( disable : 4355 )

View::View( eq::Layout* parent )
        : eq::View( parent )
        , _proxy( this )
        , _modelID( co::base::UUID::ZERO )
        , _idleSteps( 0 )
{
    setUserData( &_proxy );
}

#pragma warning( pop )

View::~View()
{
    setUserData( 0 );
    _modelID = co::base::UUID::ZERO;
    _idleSteps = 0;
}

void View::Proxy::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_MODEL )
        os << _view->_modelID;
    if( dirtyBits & DIRTY_IDLE )
        os << _view->_idleSteps;
}

void View::Proxy::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
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

void View::setModelID( const co::base::uint128_t& id )
{
    if( _modelID == id )
        return;

    _modelID = id;
    _proxy.setDirty( Proxy::DIRTY_MODEL );
}

void View::setIdleSteps( const uint32_t steps )
{
    if( _idleSteps == steps )
        return;

    _idleSteps = steps;
    _proxy.setDirty( Proxy::DIRTY_IDLE );
}

}

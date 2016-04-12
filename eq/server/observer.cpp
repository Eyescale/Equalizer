
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "observer.h"

#include "config.h"
#include "global.h"
#include "view.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{

typedef fabric::Observer< Config, Observer > Super;

Observer::Observer( Config* parent )
        : Super( parent )
        , _state( STATE_ACTIVE )
{
    _updateEyes();
}

Observer::~Observer()
{
}

void Observer::setDirty( const uint64_t bits )
{
    Super::setDirty( bits );
    for( ViewsCIter i = _views.begin(); i != _views.end(); ++i )
        (*i)->setDirty( View::DIRTY_OBSERVER );
}

void Observer::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );

    if( dirtyBits & ( DIRTY_EYE_POSITION | DIRTY_HEAD ))
        _updateEyes();
    if( dirtyBits & DIRTY_FOCUS ||
        ( (dirtyBits & DIRTY_HEAD) && getFocusMode() != FOCUSMODE_FIXED ))
    {
        _updateViews();
    }
    if( dirtyBits & DIRTY_HEAD )
        _inverseHeadMatrix = getHeadMatrix().inverse();
}

ServerPtr Observer::getServer()
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

void Observer::addView( View* view )
{
    LBASSERT( lunchbox::find( _views, view ) == _views.end( ));
    _views.push_back( view );
}

void Observer::removeView( View* view )
{
    ViewsIter i = lunchbox::find( _views, view );
    LBASSERT( i != _views.end( ));
    if( i != _views.end( ))
        _views.erase( i );
}

void Observer::init()
{
    _updateEyes();
    _updateViews();
    _inverseHeadMatrix = getHeadMatrix().inverse();
}

void Observer::_updateEyes()
{
    const Matrix4f& head = getHeadMatrix();
    for( size_t i = 0; i < NUM_EYES; ++i )
        _eyeWorld[ i ] = head * getEyePosition( Eye( 1 << i ));

    LBVERB << "Eye position: " << _eyeWorld[ fabric::EYE_CYCLOP_BIT ]
           << std::endl;
}

void Observer::_updateViews()
{
    for( ViewsIter i = _views.begin(); i != _views.end(); ++i )
        (*i)->updateFrusta();
}

void Observer::postDelete()
{
    _state = STATE_DELETE;
    getConfig()->postNeedsFinish();
}

}
}
#include "../fabric/observer.ipp"
template class eq::fabric::Observer< eq::server::Config, eq::server::Observer >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
      const eq::fabric::Observer< eq::server::Config, eq::server::Observer >& );
/** @endcond */

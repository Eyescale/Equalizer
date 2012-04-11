
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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
        , _inverseHeadMatrix( Matrix4f::IDENTITY )
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

    if( dirtyBits & ( DIRTY_EYE_BASE | DIRTY_HEAD ))
        _updateEyes();
    if( dirtyBits & DIRTY_FOCUS ||
        ( (dirtyBits & DIRTY_HEAD) && getFocusMode() != FOCUSMODE_FIXED ))
    {
        _updateViews();
    }
    if( dirtyBits & DIRTY_HEAD )
        getHeadMatrix().inverse( _inverseHeadMatrix );
}

ServerPtr Observer::getServer() 
{
    Config* config = getConfig();
    LBASSERT( config );
    return ( config ? config->getServer() : 0 );
}

void Observer::addView( View* view )
{
    LBASSERT( stde::find( _views, view ) == _views.end( ));
    _views.push_back( view );
}

void Observer::removeView( View* view )
{
    ViewsIter i = stde::find( _views, view );
    LBASSERT( i != _views.end( ));
    if( i != _views.end( ))
        _views.erase( i );
}

void Observer::init()
{
    _updateEyes();
    _updateViews();
    getHeadMatrix().inverse( _inverseHeadMatrix );
}
    
void Observer::_updateEyes()
{
    const float eyeBase_2 = .5f * getEyeBase();
    const Matrix4f& head = getHeadMatrix();

    // eye_world = (+-eye_base/2., 0, 0 ) x head_matrix
    // OPT: don't use vector operator* due to possible simplification
    const int32_t cyclop = lunchbox::getIndexOfLastBit( eq::EYE_CYCLOP );
    const int32_t right  = lunchbox::getIndexOfLastBit( eq::EYE_RIGHT );
    const int32_t left   = lunchbox::getIndexOfLastBit( eq::EYE_LEFT );

    _eyes[ cyclop ].x() = head.at( 0, 3 );
    _eyes[ cyclop ].y() = head.at( 1, 3 );
    _eyes[ cyclop ].z() = head.at( 2, 3 );
    _eyes[ cyclop ]    /= head.at( 3, 3 );

    _eyes[ left ].x() = ( -eyeBase_2 * head.at( 0, 0 ) + head.at( 0, 3 ));
    _eyes[ left ].y() = ( -eyeBase_2 * head.at( 1, 0 ) + head.at( 1, 3 ));
    _eyes[ left ].z() = ( -eyeBase_2 * head.at( 2, 0 ) + head.at( 2, 3 ));
    _eyes[ left ]    /= ( -eyeBase_2 * head.at( 3, 0 ) + head.at( 3, 3 ));

    _eyes[ right ].x() = ( eyeBase_2 * head.at( 0, 0 ) + head.at( 0, 3 ));
    _eyes[ right ].y() = ( eyeBase_2 * head.at( 1, 0 ) + head.at( 1, 3 ));
    _eyes[ right ].z() = ( eyeBase_2 * head.at( 2, 0 ) + head.at( 2, 3 ));
    _eyes[ right ]    /= ( eyeBase_2 * head.at( 3, 0 ) + head.at( 3, 3 ));

    EQVERB << "Eye position: " << _eyes[ cyclop ] << std::endl;
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

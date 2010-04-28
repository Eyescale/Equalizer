
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

#include "observer.h"

#include "config.h"
#include "global.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{
namespace server
{

typedef fabric::Observer< Config, Observer > Super;

Observer::Observer( Config* parent )
        : Super( parent )
        , _inverseHeadMatrix( Matrix4f::IDENTITY )
{
#ifdef EQ_USE_DEPRECATED
    setEyeBase(Global::instance()->getConfigFAttribute(Config::FATTR_EYE_BASE));
#endif
    _updateEyes();
}

Observer::~Observer()
{
}

void Observer::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );

    if( dirtyBits & ( DIRTY_EYE_BASE | DIRTY_HEAD ))
        _updateEyes();
    if( dirtyBits & DIRTY_HEAD )
        getHeadMatrix().inverse( _inverseHeadMatrix );
}

void Observer::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    EQASSERT( getID() != EQ_ID_INVALID );

    session->unmapObject( this );
}

void Observer::init()
{
    _updateEyes();
    getHeadMatrix().inverse( _inverseHeadMatrix );
}
    
void Observer::_updateEyes()
{
    const float eyeBase_2 = .5f * getEyeBase();
    const Matrix4f& head = getHeadMatrix();

    // eye_world = (+-eye_base/2., 0, 0 ) x head_matrix
    // OPT: don't use vector operator* due to possible simplification

    _eyes[eq::EYE_CYCLOP].x() = head.at( 0, 3 );
    _eyes[eq::EYE_CYCLOP].y() = head.at( 1, 3 );
    _eyes[eq::EYE_CYCLOP].z() = head.at( 2, 3 );
    _eyes[eq::EYE_CYCLOP]    /= head.at( 3, 3 );

    _eyes[eq::EYE_LEFT].x() = ( -eyeBase_2 * head.at( 0, 0 ) + head.at( 0, 3 ));
    _eyes[eq::EYE_LEFT].y() = ( -eyeBase_2 * head.at( 1, 0 ) + head.at( 1, 3 ));
    _eyes[eq::EYE_LEFT].z() = ( -eyeBase_2 * head.at( 2, 0 ) + head.at( 2, 3 ));
    _eyes[eq::EYE_LEFT]    /= ( -eyeBase_2 * head.at( 3, 0 ) + head.at( 3, 3 ));

    _eyes[eq::EYE_RIGHT].x() = ( eyeBase_2 * head.at( 0, 0 ) + head.at( 0, 3 ));
    _eyes[eq::EYE_RIGHT].y() = ( eyeBase_2 * head.at( 1, 0 ) + head.at( 1, 3 ));
    _eyes[eq::EYE_RIGHT].z() = ( eyeBase_2 * head.at( 2, 0 ) + head.at( 2, 3 ));
    _eyes[eq::EYE_RIGHT]    /= ( eyeBase_2 * head.at( 3, 0 ) + head.at( 3, 3 ));

    EQVERB << "Eye position: " << _eyes[ eq:: EYE_CYCLOP ] << std::endl;
}

}
}
#include "../lib/fabric/observer.ipp"
template class eq::fabric::Observer< eq::server::Config, eq::server::Observer >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
      const eq::fabric::Observer< eq::server::Config, eq::server::Observer >& );
/** @endcond */

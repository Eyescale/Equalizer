
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
#include "leafVisitor.h"
#include "paths.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{

template< typename C, typename O >
Observer< C, O >::Observer( C* config )
        : _config( config )
{
    EQASSERT( config );
    config->_addObserver( static_cast< O* >( this ));
    _data.eyeBase = config->getFAttribute( C::FATTR_EYE_BASE );
    EQINFO << "New " << base::className( this ) << std::endl;
}

template< typename C, typename O >
Observer< C, O >::~Observer()
{
    EQINFO << "Delete " << base::className( this ) << std::endl;
    _config->_removeObserver( static_cast< O* >( this ));
}

template< typename C, typename O >
void Observer< C, O >::backup()
{
    Object::backup();
    _backup = _data;
}

template< typename C, typename O >
void Observer< C, O >::restore()
{
    _data = _backup;
    Object::restore();
    setDirty( DIRTY_EYE_BASE | DIRTY_HEAD );
}

template< typename C, typename O >
void Observer< C, O >::serialize( net::DataOStream& os, 
                                  const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        os << _data.eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        os << _data.headMatrix;
}

template< typename C, typename O >
void Observer< C, O >::deserialize( net::DataIStream& is,
                                    const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        is >> _data.eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        is >> _data.headMatrix;
}

template< typename C, typename O >
void Observer< C, O >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( C::DIRTY_OBSERVERS );
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( LeafVisitor< O >& visitor )
{
    return visitor.visit( static_cast< O* >( this ));
}

template< typename C, typename O >
VisitorResult Observer< C, O >::accept( LeafVisitor< O >& visitor ) const
{
    return visitor.visit( static_cast< const O* >( this ));
}

template< typename C, typename O >
ObserverPath Observer< C, O >::getPath() const
{
    const ObserverVector&  observers = _config->getObservers();
    typename ObserverVector::const_iterator i = std::find( observers.begin(), 
                                                           observers.end(),
                                                           this );
    EQASSERT( i != observers.end( ));

    ObserverPath path;
    path.observerIndex = std::distance( observers.begin(), i );
    return path;
}

template< typename C, typename O >
void Observer< C, O >::setEyeBase( const float eyeBase )
{
    _data.eyeBase = eyeBase;
    setDirty( DIRTY_EYE_BASE );
}

template< typename C, typename O >
void Observer< C, O >::setHeadMatrix( const Matrix4f& matrix )
{
    _data.headMatrix = matrix;
    setDirty( DIRTY_HEAD );
}

template< typename C, typename O >
std::ostream& operator << ( std::ostream& os, const Observer< C, O >& observer )
{
    os << base::disableFlush << base::disableHeader << "observer" << std::endl;
    os << "{" << std::endl << base::indent; 

    const std::string& name = observer.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const float eyeBase = observer.getEyeBase();
    if( eyeBase != 0.05f /* TODO use Config::FATTR_EYE_BASE */ )
        os << eyeBase << std::endl;

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}


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

#include "observer.h"

#include "config.h"
#include "observerVisitor.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{

Observer::Observer()
        : _config( 0 )
        , _eyeBase( 0.05f )
        , _headMatrix( Matrix4f::IDENTITY )
{
}

Observer::~Observer()
{
    EQASSERT( !_config );
}

void Observer::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        os << _eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        os << _headMatrix;
}

void Observer::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_EYE_BASE )
        is >> _eyeBase;
    if( dirtyBits & DIRTY_HEAD )
        is >> _headMatrix;
}

VisitorResult Observer::accept( ObserverVisitor& visitor )
{
    return visitor.visit( this );
}

VisitorResult Observer::accept( ObserverVisitor& visitor ) const
{
    return visitor.visit( this );
}

void Observer::deregister()
{
    EQASSERT( _config );
    EQASSERT( isMaster( ));

    _config->deregisterObject( this );
}

void Observer::setEyeBase( const float eyeBase )
{
    _eyeBase = eyeBase;
    setDirty( DIRTY_EYE_BASE );
}

void Observer::setHeadMatrix( const Matrix4f& matrix )
{
    _headMatrix = matrix;
    setDirty( DIRTY_HEAD );
}

std::ostream& operator << ( std::ostream& os, const Observer* observer )
{
    if( !observer )
        return os;
    
    os << disableFlush << disableHeader << "observer" << std::endl;
    os << "{" << std::endl << indent; 

    const std::string& name = observer->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const float eyeBase = observer->getEyeBase();
    if( eyeBase != 0.05f /* TODO use Config::FATTR_EYE_BASE */ )
        os << eyeBase << std::endl;

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}


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

#include "objectVersion.h"

#include "object.h"
#include <eq/base/idPool.h>

namespace eq
{
namespace net
{
ObjectVersion NONE;

ObjectVersion::ObjectVersion()
        : id( EQ_ID_INVALID ), version( Object::VERSION_NONE )
{}

ObjectVersion::ObjectVersion( const uint32_t id_, const uint32_t version_ )
        : id( id_ ), version( version_ )
{}

ObjectVersion::ObjectVersion( const Object* object )
        : id( object->getID( )), 
          version( object->getVersion( )) 
{
}

ObjectVersion& ObjectVersion::operator = ( const Object* object )
{
    if( object )
    {
        id = object->getID();
        version = object->getVersion();
    }
    else
    {
        id = EQ_ID_INVALID;
        version = Object::VERSION_NONE;
    }

    return *this;
}

}
}

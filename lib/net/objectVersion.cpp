
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

#include "objectVersion.h"

#include "object.h"
#include <eq/base/idPool.h>

namespace eq
{
namespace net
{
ObjectVersion NONE;

ObjectVersion::ObjectVersion()
        : identifier( EQ_ID_NONE ), version( VERSION_NONE )
{}

ObjectVersion::ObjectVersion( const uint32_t id_, const uint32_t version_ )
        : identifier( id_ ), version( version_ )
{}

ObjectVersion::ObjectVersion( const Object* object )
        : identifier( EQ_ID_NONE ), version( VERSION_NONE )
{
    if( object && object->isAttached( ))
    {
        identifier = object->getID();
        version = object->getVersion(); 
    }
}

ObjectVersion& ObjectVersion::operator = ( const Object* object )
{
    if( object )
    {
        identifier = object->getID();
        version = object->getVersion();
    }
    else
    {
        identifier = EQ_ID_NONE;
        version = VERSION_NONE;
    }

    return *this;
}

}
}

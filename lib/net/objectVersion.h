
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

#ifndef EQNET_OBJECTVERSION_H
#define EQNET_OBJECTVERSION_H

#include <eq/base/base.h>

#include <iostream>

namespace eq
{
namespace net
{
    class Object;

    /** A helper struct bundling an object identifier and version. */
    struct ObjectVersion
    {
        EQ_EXPORT ObjectVersion();
        EQ_EXPORT ObjectVersion( const uint32_t id, const uint32_t version );
        EQ_EXPORT ObjectVersion( const Object* object );
        EQ_EXPORT ObjectVersion& operator = ( const Object* object );
        
        uint32_t id;
        uint32_t version;
    };

    inline std::ostream& operator << (std::ostream& os, const ObjectVersion& ov)
    {
        os << " id " << ov.id << " v" << ov.version;
        return os;
    }
}
}

#endif // EQNET_OBJECT_H

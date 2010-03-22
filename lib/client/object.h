
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

#ifndef EQ_OBJECT_H
#define EQ_OBJECT_H

#include <eq/fabric/object.h>        // base class

namespace eq
{
    /**
     * Base class for all distributed, inheritable objects in the eq namespace.
     *
     * This class implements one usage pattern of net::Object, which allows
     * sub-classing and serialization of distributed Objects used by
     * Equalizer. The inheritance Object -> Frustum -> View illustrates
     * the usage of eq::Object.
     */
    class Object : public fabric::Object // TODO retire?
    {
    public:
        /** Construct a new Object. */
        Object() {}
        
        /** Destruct the object. */
        virtual ~Object() {}
    };
}
#endif // EQ_OBJECT_H

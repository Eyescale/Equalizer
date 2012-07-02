
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef CO_OBJECTFACTORY_H
#define CO_OBJECTFACTORY_H

#include <co/object.h>       // deleted inline

namespace co
{
    enum ObjectType
    {
        OBJECTTYPE_NONE,
        OBJECTTYPE_CUSTOM = 16
    };

    /** The interface to create objects, used by objectMap. */
    class ObjectFactory
    {
    public:
        /** @internal Construct a new object factory. */
        ObjectFactory(){}

        /** @internal Destruct this object factory. */
        virtual ~ObjectFactory(){}

        /**
         * @return a new object instance of the given type.
         * @version 0.5.1
         * @sa ObjectType, Config::getObject(), Renderer::getObject() */
        virtual co::Object* createObject( const uint32_t type )
            { LBUNIMPLEMENTED; return 0; }

        /** Delete the given object of the given type. @version 0.5.1 */
        virtual void destroyObject( co::Object* object, const uint32_t type )
            { delete object; }
    };
}
#endif // CO_OBJECTFACTORY_H

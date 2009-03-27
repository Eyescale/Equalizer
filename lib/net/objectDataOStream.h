
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQNET_OBJECTDATAOSTREAM_H
#define EQNET_OBJECTDATAOSTREAM_H

#include <eq/net/dataOStream.h>   // base class
#include <eq/net/object.h>        // used in inline constructor

namespace eq
{
namespace net
{
    class Object;

    /**
     * The DataOStream for object data.
     */
    class EQ_EXPORT ObjectDataOStream : public DataOStream
    {
    public:
        ObjectDataOStream( const Object* object )
            : _object( object ), _version( Object::VERSION_NONE )
            , _instanceID( EQ_ID_ANY )
        {}

        virtual ~ObjectDataOStream(){}
 
        void setVersion( const uint32_t version ) { _version = version; }
        uint32_t getVersion() const { return _version; }

        void setInstanceID( const uint32_t instanceID )
            { _instanceID = instanceID; }
        uint32_t getInstanceID() const { return _instanceID; }

    protected:
        const Object* _object;
        uint32_t      _version;
        uint32_t      _instanceID;
    };
}
}
#endif //EQNET_OBJECTDATAOSTREAM_H

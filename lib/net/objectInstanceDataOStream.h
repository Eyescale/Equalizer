
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

#ifndef EQNET_OBJECTINSTANCEDATAOSTREAM_H
#define EQNET_OBJECTINSTANCEDATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace eq
{
namespace net
{
    class Object;

    /**
     * The DataOStream for object instance data.
     */
    class EQ_EXPORT ObjectInstanceDataOStream : public ObjectDataOStream
    {
    public:
        ObjectInstanceDataOStream( const Object* object );
        virtual ~ObjectInstanceDataOStream();
 
    protected:
        virtual void sendHeader( const void* buffer, const uint64_t size )
            { _sequence = 0; sendBuffer( buffer, size ); }
        virtual void sendBuffer( const void* buffer, const uint64_t size );
        virtual void sendFooter( const void* buffer, const uint64_t size );
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { _sequence = 0; sendFooter( buffer, size ); }

    private:
        uint32_t _sequence;
    };
}
}
#endif //EQNET_OBJECTINSTANCEDATAOSTREAM_H

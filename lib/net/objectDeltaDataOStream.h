
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTDELTADATAOSTREAM_H
#define EQNET_OBJECTDELTADATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace eq
{
namespace net
{
    class Object;

    /**
     * The DataOStream for object delta version data.
     */
    class EQ_EXPORT ObjectDeltaDataOStream : public ObjectDataOStream
    {
    public:
        ObjectDeltaDataOStream( const Object* object );
        virtual ~ObjectDeltaDataOStream();

    protected:
        virtual void sendHeader( const void* buffer, const uint64_t size )
            { sendBuffer( buffer, size ); }
        virtual void sendBuffer( const void* buffer, const uint64_t size );
        virtual void sendFooter( const void* buffer, const uint64_t size );
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { sendFooter( buffer, size ); }
    };
}
}
#endif //EQNET_OBJECTDELTADATAOSTREAM_H

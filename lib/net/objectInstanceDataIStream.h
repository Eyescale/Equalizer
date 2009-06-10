
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTINSTANCEDATAISTREAM_H
#define EQNET_OBJECTINSTANCEDATAISTREAM_H

#include "objectDataIStream.h"   // base class

#include <deque>

namespace eq
{
namespace net
{
    class Command;

    /**
     * The DataIStream for object instance data.
     */
    class ObjectInstanceDataIStream : public ObjectDataIStream
    {
    public:
        ObjectInstanceDataIStream();
        virtual ~ObjectInstanceDataIStream();

    protected:
        virtual bool getNextBuffer( const uint8_t** buffer, uint64_t* size );

    private:
        uint32_t _sequence;
    };
}
}
#endif //EQNET_OBJECTINSTANCEDATAISTREAM_H

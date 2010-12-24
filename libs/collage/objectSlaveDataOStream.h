
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef CO_OBJECTSLAVEDATAOSTREAM_H
#define CO_OBJECTSLAVEDATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace co
{
    class Object;
    struct ObjectSlaveDeltaPacket;

    /**
     * The DataOStream for object slave version data.
     */
    class ObjectSlaveDataOStream : public ObjectDataOStream
    {
    public:
        ObjectSlaveDataOStream( const ObjectCM* cm );
        virtual ~ObjectSlaveDataOStream();

    protected:
        virtual void sendData( const uint32_t name, const uint32_t nChunks,
                               const void* const* chunks,
                               const uint64_t* chunkSizes,
                               const uint64_t sizeUncompressed );

        virtual void sendFooter( const uint32_t name, const uint32_t nChunks,
                                 const void* const* chunks, 
                                 const uint64_t* chunkSizes,
                                 const uint64_t sizeUncompressed );

    private:
        co::base::UUID _commit;
    };
}
#endif //CO_OBJECTSLAVEDATAOSTREAM_H


/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef CO_OBJECTDELTADATAOSTREAM_H
#define CO_OBJECTDELTADATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace co
{
    /** The DataOStream for object delta version data. */
    class ObjectDeltaDataOStream : public ObjectDataOStream
    {
    public:
        ObjectDeltaDataOStream( const ObjectCM* cm );
        virtual ~ObjectDeltaDataOStream();

    protected:
        virtual void sendData( const void* buffer, const uint64_t size,
                               const bool last );
    };
}
#endif //CO_OBJECTDELTADATAOSTREAM_H

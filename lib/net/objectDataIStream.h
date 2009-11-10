
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTDATAISTREAM_H
#define EQNET_OBJECTDATAISTREAM_H

#include <eq/net/dataIStream.h>   // base class
#include <eq/net/version.h>       // enum
#include <eq/base/monitor.h>      // member

#include <deque>

namespace eq
{
namespace net
{
    class Command;

    /**
     * The DataIStream for object data.
     */
    class ObjectDataIStream : public DataIStream
    {
    public:
        ObjectDataIStream();
        ObjectDataIStream( const ObjectDataIStream& from );
        virtual ~ObjectDataIStream();

        void addDataPacket( Command& command );
        size_t getDataSize() const;

        virtual uint32_t getVersion() const { return _version.get(); }
        uint32_t getPendingVersion() const;

        void waitReady() const { _version.waitNE( VERSION_INVALID ); }
        bool isReady() const { return _version != VERSION_INVALID; }

        virtual size_t nRemainingBuffers() const  { return _commands.size(); }

        virtual void reset();

        enum Type
        {
            TYPE_INSTANCE,
            TYPE_DELTA
        };
        virtual Type getType() const = 0;

    protected:
        const Command* getNextCommand();

    private:
        /** All data command packets for this istream. */
        CommandDeque _commands;

        /** The object version associated with this input stream. */
        base::Monitor< uint32_t > _version;

        void _setReady() { _version = getPendingVersion(); }
    };
}
}
#endif //EQNET_OBJECTDATAISTREAM_H

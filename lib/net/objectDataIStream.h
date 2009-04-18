
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTDATAISTREAM_H
#define EQNET_OBJECTDATAISTREAM_H

#include <eq/net/dataIStream.h>   // base class
#include <eq/net/object.h>        // nested enum
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
    class EQ_EXPORT ObjectDataIStream : public DataIStream
    {
    public:
        ObjectDataIStream();
        virtual ~ObjectDataIStream();

        void addDataPacket( Command& command );

        void setVersion( const uint32_t version ) { _version = version; }
        virtual uint32_t getVersion() const       { return _version.get(); }
        void waitReady() const { _version.waitNE( Object::VERSION_INVALID ); }

        virtual size_t nRemainingBuffers() const  { return _commands.size(); }

        virtual void reset();

    protected:
        const Command* getNextCommand();

    private:
        /** All data command packets for this istream. */
        std::deque< Command* >      _commands;

        /** The last returned, to be released command. */
        Command*                    _lastCommand;

        /** The object version associated with this input stream. */
        base::Monitor< uint32_t > _version;
    };
}
}
#endif //EQNET_OBJECTDATAISTREAM_H

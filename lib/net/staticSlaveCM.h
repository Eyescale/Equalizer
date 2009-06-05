
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

#ifndef EQNET_STATICSLAVECM_H
#define EQNET_STATICSLAVECM_H

#include <eq/net/objectCM.h>     // base class
#include <eq/net/command.h>      // member
#include <eq/net/object.h>       // nested enum (Object::Version)
#include <eq/base/idPool.h>      // for EQ_ID_INVALID
#include <eq/base/monitor.h>     // member

namespace eq
{
namespace net
{
    class Node;
    class ObjectDataIStream;

    /** 
     * An object change manager handling static object slave instances.
     */
    class StaticSlaveCM : public ObjectCM
    {
    public:
        StaticSlaveCM( Object* object );
        virtual ~StaticSlaveCM();

        virtual void makeThreadSafe(){}

        /**
         * @name Versioning
         */
        //*{
        virtual uint32_t commitNB() { EQDONTCALL; return EQ_ID_INVALID; }
        virtual uint32_t commitSync( const uint32_t commitID )
            { EQDONTCALL; return Object::VERSION_NONE; }

        virtual void obsolete( const uint32_t version ) { EQDONTCALL; }

        virtual void setAutoObsolete( const uint32_t count,
                                      const uint32_t flags ) { EQDONTCALL; }
        virtual uint32_t getAutoObsoleteCount() const
            { EQDONTCALL; return 0; }

        virtual uint32_t sync( const uint32_t version )
            { EQDONTCALL; return Object::VERSION_NONE; }

        virtual uint32_t getHeadVersion() const { return Object::VERSION_NONE; }
        virtual uint32_t getVersion() const     { return Object::VERSION_NONE; }
        virtual uint32_t getOldestVersion() const {return Object::VERSION_NONE;}
        //*}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const { return EQ_ID_INVALID; }

        virtual void addSlave( NodePtr slave, const uint32_t instanceID,
                               const uint32_t version ) { EQDONTCALL; }
        virtual void removeSlave( NodePtr node ) { EQDONTCALL; }
        virtual void addOldMaster( NodePtr node, const uint32_t instanceID )
            { EQDONTCALL }

        virtual void applyMapData();

    protected:
        /** The managed object. */
        Object* _object;

        /** istream for receiving the current version */
        ObjectDataIStream* _currentIStream;

    private:
        /* The command handlers. */
        CommandResult _cmdInstanceData( Command& command );
        CommandResult _cmdInstance( Command& command );
    };
}
}

#endif // EQNET_STATICSLAVECM_H


/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_MASTERCM_H
#define CO_MASTERCM_H

#include "objectCM.h" // base class
#include "dataIStreamQueue.h" // member
#include <co/types.h>

#include <lunchbox/mtQueue.h> // member
#include <lunchbox/pool.h>    // member
#include <lunchbox/stdExt.h>  // member
#include <lunchbox/thread.h>  // thread-safety check

namespace co
{
    /** 
     * @internal
     * The base class for versioned master change managers.
     */
    class MasterCM : public ObjectCM
    {
    protected:
        typedef lunchbox::ScopedWrite Mutex;

    public:
        MasterCM( Object* object );
        virtual ~MasterCM();

        virtual void init(){}

        /** @name Versioning */
        //@{
        virtual uint128_t sync( const uint128_t& version );

        virtual uint128_t getHeadVersion() const
            { Mutex mutex( _slaves ); return _version; }
        virtual uint128_t getVersion() const
            { Mutex mutex( _slaves ); return _version; }
        //@}

        virtual bool isMaster() const { return true; }
        virtual uint32_t getMasterInstanceID() const
            { LBDONTCALL; return EQ_INSTANCE_INVALID; }

        virtual void addSlave( Command& command );
        virtual void removeSlave( NodePtr node );
        virtual void removeSlaves( NodePtr node );
        virtual const Nodes getSlaveNodes() const
            { Mutex mutex( _slaves ); return *_slaves; }

    protected:
        /** The list of subscribed slave nodes. */
        lunchbox::Lockable< Nodes > _slaves;

        /** The current version. */
        uint128_t _version;

    private:
        typedef stde::hash_map< uint128_t, uint32_t > SlavesCount;
        /** The number of object instances subscribed per slave node. */
        SlavesCount _slavesCount;

        /** Slave commit queue. */
        DataIStreamQueue _slaveCommits;

        virtual void _addSlave( NodePtr node );

        uint128_t _apply( ObjectDataIStream* is );

        /* The command handlers. */
        bool _cmdSlaveDelta( Command& command );
        bool _cmdDiscard( Command& ) { return true; }

        LB_TS_VAR( _cmdThread );
        LB_TS_VAR( _rcvThread );
    };
}

#endif // CO_MASTERCM_H

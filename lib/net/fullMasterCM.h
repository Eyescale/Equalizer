
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_FULLMASTERCM_H
#define EQNET_FULLMASTERCM_H

#include "masterCM.h"        // base class
#include "objectInstanceDataOStream.h" // member

#include <deque>

namespace eq
{
namespace net
{
    class Node;
    class ObjectDataIStream;

    /** 
     * An object change manager handling only full versions for the master
     * instance.
     * @internal
     */
    class FullMasterCM : public MasterCM
    {
    public:
        FullMasterCM( Object* object );
        virtual ~FullMasterCM();

        /**
         * @name Versioning
         */
        //@{
        virtual void obsolete( const uint32_t version ) { EQUNIMPLEMENTED; }

        virtual void setAutoObsolete( const uint32_t count, 
                                      const uint32_t flags )
            { _nVersions = count; _obsoleteFlags = flags; }
        
        virtual uint32_t getAutoObsoleteCount() const { return _nVersions; }
        virtual uint32_t getOldestVersion() const;
        //@}

        virtual uint32_t addSlave( Command& command );
        virtual void removeSlave( NodePtr node );

    protected:
        /** The number of commits, needed for auto-obsoletion. */
        uint32_t _commitCount;

        /** The number of old versions to retain. */
        uint32_t _nVersions;

        /** The flags for automatic version obsoletion. */
        uint32_t _obsoleteFlags;

        struct InstanceData
        {
            InstanceData( const Object* object ) 
                    : os( object ), commitCount(0) {}

            ObjectInstanceDataOStream os;
            uint32_t commitCount;
        };
        
        typedef std::deque< InstanceData* > InstanceDataDeque;
        typedef std::vector< InstanceData* > InstanceDataVector;

        /** The list of full instance datas, head version last. */
        InstanceDataDeque _instanceDatas;
        InstanceDataVector _instanceDataCache;

        InstanceData* _newInstanceData();

        void _obsolete();
        void _checkConsistency() const;

        /* The command handlers. */
        CommandResult _cmdCommit( Command& command );
    };
}
}

#endif // EQNET_FULLMASTERCM_H


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

#ifndef EQNET_UNBUFFEREDMASTERCM_H
#define EQNET_UNBUFFEREDMASTERCM_H

#include "masterCM.h"           // base class

namespace co
{
    class Node;

    /** 
     * An object change manager handling versioned objects without any
     * buffering.
     * @internal
     */
    class UnbufferedMasterCM : public MasterCM
    {
    public:
        UnbufferedMasterCM( Object* object );
        virtual ~UnbufferedMasterCM();

        /**
         * @name Versioning
         */
        //@{
        virtual void setAutoObsolete( const uint32_t ) { EQDONTCALL; } 
        virtual uint32_t getAutoObsolete() const { EQDONTCALL; return 0; }
        virtual uint128_t getOldestVersion() const { return _version; }
        //@}

        virtual uint128_t addSlave( Command& command );
        virtual void removeSlave( NodePtr node );

    private:
        /* The command handlers. */
        bool _cmdCommit( Command& pkg );
    };
}

#endif // CO_UNBUFFEREDMASTERCM_H

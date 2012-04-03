
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_NULLCM_H
#define EQNET_NULLCM_H

#include "objectCM.h"    // base class

namespace co
{
    class Node;

    /** 
     * The NOP object change manager for unmapped objects.
     * @internal
     */
    class NullCM : public ObjectCM
    {
    public:
        NullCM() : ObjectCM( 0 ) {}
        virtual ~NullCM() {}

        virtual void init() {}

        virtual void push( const uint128_t&, const uint128_t&, Nodes& )
            { EQDONTCALL; }

        virtual uint128_t getHeadVersion() const   { return VERSION_NONE; }
        virtual uint128_t getVersion() const       { return VERSION_NONE; }
        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const
            { EQDONTCALL; return EQ_INSTANCE_INVALID; }

        virtual void addSlave( Command& command ) { EQDONTCALL; }
        virtual void removeSlaves( NodePtr ) { EQDONTCALL; }

    private:
    };
}

#endif // EQNET_NULLCM_H

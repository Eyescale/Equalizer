
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

#ifndef CO_STATICSLAVECM_H
#define CO_STATICSLAVECM_H

#include "objectCM.h"     // base class
#include <co/objectVersion.h> // VERSION_FOO values
#include <lunchbox/thread.h>  // thread-safety check

namespace co
{
    /** @internal
     * An object change manager handling static object slave instances.
     */
    class StaticSlaveCM : public ObjectCM
    {
    public:
        StaticSlaveCM( Object* object );
        virtual ~StaticSlaveCM();

        virtual void init(){}

        /**
         * @name Versioning
         */
        //@{
        virtual uint128_t getHeadVersion() const { return VERSION_FIRST; }
        virtual uint128_t getVersion() const     { return VERSION_FIRST; }
        //@}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const
            { return EQ_INSTANCE_INVALID; }

        virtual void removeSlaves( NodePtr ) {}

        virtual void applyMapData( const uint128_t& version );
        virtual void addInstanceDatas( const ObjectDataIStreamDeque&, 
                                       const uint128_t& startVersion );
    protected:
        /** input stream for receiving the current version */
        ObjectDataIStream* _currentIStream;

    private:
        /* The command handlers. */
        bool _cmdInstance( Command& command );
        LB_TS_VAR( _rcvThread );
    };
}

#endif // CO_STATICSLAVECM_H


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

#ifndef CO_STATICSLAVECM_H
#define CO_STATICSLAVECM_H

#include <co/objectCM.h>     // base class
#include <co/version.h>      // enum

namespace co
{
    /** 
     * An object change manager handling static object slave instances.
     * @internal
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
        virtual uint32_t commitNB() { EQDONTCALL; return EQ_UNDEFINED_UINT32; }
        virtual uint128_t commitSync( const uint32_t )
            { EQDONTCALL; return VERSION_NONE; }

        virtual void setAutoObsolete( const uint32_t ) { EQDONTCALL; }
        virtual uint32_t getAutoObsolete() const { EQDONTCALL; return 0; }

        virtual uint128_t sync( const uint128_t& )
            { EQDONTCALL; return VERSION_NONE; }

        virtual uint128_t getHeadVersion() const { return VERSION_NONE; }
        virtual uint128_t getVersion() const     { return VERSION_NONE; }
        virtual uint128_t getOldestVersion() const { return VERSION_NONE; }
        //@}

        virtual bool isMaster() const { return false; }
        virtual uint32_t getMasterInstanceID() const
            { return EQ_INSTANCE_INVALID; }

        virtual uint128_t addSlave( Command& )
            { EQDONTCALL; return VERSION_INVALID; }
        virtual void removeSlave( NodePtr ) { EQDONTCALL; }

        virtual void applyMapData( const uint128_t& version );
        virtual void addInstanceDatas( const ObjectDataIStreamDeque&, 
                                       const uint128_t& startVersion );
        virtual const Object* getObject( ) const { return _object; }
    protected:
        /** The managed object. */
        Object* _object;

        /** input stream for receiving the current version */
        ObjectDataIStream* _currentIStream;

    private:
        /* The command handlers. */
        bool _cmdInstance( Command& command );
    };
}

#endif // CO_STATICSLAVECM_H

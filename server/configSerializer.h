
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CONFIGSERIALIZER_H
#define EQSERVER_CONFIGSERIALIZER_H

#include "configVisitor.h"        // base class
#include "types.h"

#include <eq/net/object.h> // base class

namespace eq
{
namespace server
{
    /** 
     * Serializes all data for a Config, which is a net::Session and can't be a
     * net::Object at the same time.
     */
    class ConfigSerializer : public net::Object
    {
    public:
        ConfigSerializer( Config* const config ) : _config( config ) {}
        virtual ~ConfigSerializer() {}

    protected:
        // Use instance since then getInstanceData() is called from app thread,
        // which allows us to map all children during serialization
        virtual Object::ChangeType getChangeType() const 
            { return Object::INSTANCE; }
            
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is ) {EQDONTCALL;}

    private:
        Config* const _config;
    };
}
}
#endif // EQSERVER_CONFIGSERIALIZER_H

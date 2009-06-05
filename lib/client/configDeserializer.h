
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

#ifndef EQ_CONFIGDESERIALIZER_H
#define EQ_CONFIGDESERIALIZER_H

#include <eq/net/object.h> // base class

namespace eq
{
    class Config;

    /** Helper class to receive a config, which is a net::Session */
    class ConfigDeserializer : public net::Object
    {
    public:
        ConfigDeserializer( Config* config ) : _config( config ) {}
        virtual ~ConfigDeserializer() {}

        /** Types of the children in the serialization stream. */
        enum Type
        {
            TYPE_OBSERVER,
            TYPE_LAYOUT,
            TYPE_CANVAS,
            TYPE_LAST
        };

    protected:
        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL; }
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        Config* const _config;
    };
}

#endif // EQ_CONFIGDESERIALIZER_H

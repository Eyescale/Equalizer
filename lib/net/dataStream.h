
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#ifndef EQNET_DATASTREAM_H
#define EQNET_DATASTREAM_H

#include "objectCM.h"

#include <iostream>
#include <vector>
#include <eq/plugins/compressor.h>
#include <eq/base/global.h>
#include <eq/base/pluginRegistry.h>

namespace eq
{
namespace net
{
    class DataStream
    {
    public:
        DataStream( ) 
            : _name( EQ_COMPRESSOR_NONE )
            , _plugin( 0 ) {}
            
        DataStream( const DataStream& from )
            : _name( from._name )
            , _plugin( from._plugin ) {}
        
        virtual ~DataStream() { _plugin = 0; }

    protected:
        uint32_t getCompressorName() const { return _name; }
        eq::base::Compressor* getPlugin() const { return _plugin; }
        eq::base::Compressor* _initPlugin( const uint32_t name )
        {
            if( _name == name )
                return _plugin;

            _plugin = 0;
            _name = name;
            
            if ( name == EQ_COMPRESSOR_NONE )
                return 0;
            
            eq::base::PluginRegistry& registry = 
                                         eq::base::Global::getPluginRegistry();
            _plugin = registry.findCompressor( name );
            EQASSERT( _plugin );

            return _plugin;
        }

    private:
        uint32_t    _name;             //!< the name of the (de) compressor 
        eq::base::Compressor* _plugin; //!< Plugin handling the allocation
    };
}
}
#endif //EQNET_DATASTREAM_H

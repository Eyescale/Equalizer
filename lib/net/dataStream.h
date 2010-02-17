
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/plugins/compressor.h>
#include <eq/base/pluginRegistry.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace net
{
    class DataStream
    {
    public:
        DataStream( ) : _name( EQ_COMPRESSOR_NONE ), _plugin( 0 ) {}
            
        DataStream( const DataStream& from )
                : _name( from._name ), _plugin( from._plugin ) {}
        
        virtual ~DataStream() { _name = EQ_COMPRESSOR_NONE; _plugin = 0; }

    protected:
        uint32_t _getCompressorName() const { return _name; }
        base::Compressor* _getCompressorPlugin() const { return _plugin; }
        base::Compressor* _initCompressorPlugin( const uint32_t name );

    private:
        uint32_t _name;            //!< the name of the (de) compressor 
        base::Compressor* _plugin; //!< plugin handling the allocation
    };
}
}
#endif //EQNET_DATASTREAM_H


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
#include "dataStream.h"

#include <eq/base/global.h>

namespace eq
{
namespace net
{

base::Compressor* DataStream::_initCompressorPlugin( const uint32_t name )
{
    if( _name == name )
        return _plugin;

    _plugin = 0;
    _name = name;
            
    if ( name == EQ_COMPRESSOR_NONE )
        return 0;

    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    _plugin = registry.findCompressor( name );
    EQASSERT( _plugin );

    return _plugin;
}

}
}

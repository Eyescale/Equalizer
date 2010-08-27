
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

#include "compressor.h"

#include "compressorInfo.h"
#include "global.h"
#include "plugin.h"
#include "pluginRegistry.h"

namespace eq
{
namespace base
{

Compressor::Compressor()
        : _name( EQ_COMPRESSOR_INVALID )
        , _plugin( 0 )
        , _instance( 0 )
        , _info( 0 )
        , _isCompressor( true )
{}

Compressor::~Compressor()
{
    reset();
}

void Compressor::reset()
{
    if( _instance )
    {
        if ( _isCompressor )
           _plugin->deleteCompressor( _instance );
        else
           _plugin->deleteDecompressor( _instance );
    }

    _name = EQ_COMPRESSOR_INVALID;
    _plugin = 0;
    _instance = 0;
    _info = 0;
    _isCompressor = true;
}

float Compressor::getQuality() const
{
    return _info ? _info->quality : 1.0f;
}

Plugin* Compressor::_findPlugin( uint32_t name )
{
    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    return registry.findPlugin( name );
}

bool Compressor::isValid( uint32_t name ) const
{
    EQ_TS_THREAD( _thread );
    if( _name == EQ_COMPRESSOR_INVALID )
        return false;
    if( _name == EQ_COMPRESSOR_NONE )
        return true;

    return ( _name == name && _plugin && ( !_isCompressor || _instance ) );
}

bool Compressor::_initCompressor( uint32_t name )
{
    reset();

    if( name <= EQ_COMPRESSOR_NONE )
    {
        _name = name;
        return true;
    }

    EQ_TS_THREAD( _thread );
    _plugin = _findPlugin( name );

    EQASSERT( _plugin );
    if( !_plugin )
        return false;

    _isCompressor = true;
    _instance = _plugin->newCompressor( name );
    EQASSERT( _instance );
    
    _name = name;
    _info = &_plugin->findInfo( name );

    EQLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                        << name << std::dec << std::endl;
    return true;
}

bool Compressor::_initDecompressor( uint32_t name )
{
    reset();
    if( name <= EQ_COMPRESSOR_NONE )
    {
        _name = name;
        return true;
    }

    EQ_TS_THREAD( _thread );
    _plugin = _findPlugin( name );

    EQASSERT( _plugin );
    if( !_plugin )
        return false;

    _isCompressor = false;
    _instance = _plugin->newDecompressor( name );
    
    _name = name;
    _info = &_plugin->findInfo( name );
    EQLOG( LOG_PLUGIN ) << "Instantiated " << ( _instance ? "" : "empty " )
                        << "decompressor of type 0x" << std::hex << name
                        << std::dec << std::endl;
    return true; 
}

}
}

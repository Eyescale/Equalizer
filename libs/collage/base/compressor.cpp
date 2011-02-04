
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

namespace co
{
namespace base
{

Compressor::Compressor()
        : _name( EQ_COMPRESSOR_INVALID )
        , _plugin( 0 )
        , _instance( 0 )
        , _info( 0 )
        , _state( STATE_FREE )
{}

Compressor::~Compressor()
{
    reset();
}

void Compressor::reset()
{
    switch( _state )
    {
      case STATE_FREE:
        EQASSERT( !_instance );
        break;

      case STATE_COMPRESSOR:
        EQASSERT( _instance );
        if( _instance )
            _plugin->deleteCompressor( _instance );
        break;

      case STATE_DECOMPRESSOR:
        if( _instance )
           _plugin->deleteDecompressor( _instance );
        break;
    }

    _name = EQ_COMPRESSOR_INVALID;
    _plugin = 0;
    _instance = 0;
    _info = 0;
    _state = STATE_FREE;
}

float Compressor::getQuality() const
{
    return _info ? _info->quality : 1.0f;
}

Plugin* Compressor::_findPlugin( uint32_t name )
{
    PluginRegistry& registry = Global::getPluginRegistry();
    return registry.findPlugin( name );
}

bool Compressor::isValid( uint32_t name ) const
{
    EQ_TS_SCOPED( _thread );
    if( _name == EQ_COMPRESSOR_INVALID || _state == STATE_FREE )
        return false;
    if( _name == EQ_COMPRESSOR_NONE )
        return true;

    if( _name != name || !_plugin )
        return false;
    if( _state == STATE_DECOMPRESSOR )
        return true;

    return _instance != 0;
}

bool Compressor::initCompressor( uint32_t name )
{
    EQ_TS_SCOPED( _thread );
    EQ_TS_THREAD( _thread );

    if( name == _name )
    {
        EQASSERT( isValid( name ));
        return true;
    }

    reset();
    if( name <= EQ_COMPRESSOR_NONE )
    {
        _name = name;
        return true;
    }

    _plugin = _findPlugin( name );

    EQASSERT( _plugin );
    if( !_plugin )
        return false;

    _state = STATE_COMPRESSOR;
    _instance = _plugin->newCompressor( name );
    _name = name;
    _info = &_plugin->findInfo( name );
    EQASSERT( _instance );
    
    EQLOG( LOG_PLUGIN ) << "Instantiated compressor of type 0x" << std::hex
                        << name << std::dec << std::endl;
    return true;
}

bool Compressor::initDecompressor( uint32_t name )
{
    EQ_TS_SCOPED( _thread );
    EQ_TS_THREAD( _thread );

    if( name == _name )
    {
        EQASSERT( isValid( name ));
        return true;
    }

    reset();
    if( name <= EQ_COMPRESSOR_NONE )
    {
        _name = name;
        return true;
    }

    _plugin = _findPlugin( name );
    EQASSERT( _plugin );

    if( !_plugin )
        return false;

    _state = STATE_DECOMPRESSOR;
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

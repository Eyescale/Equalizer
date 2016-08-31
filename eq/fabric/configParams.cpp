
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "configParams.h"

#include "equalizer.h"
#include "global.h"

#include <co/dataOStream.h>
#include <co/dataIStream.h>
#include <co/global.h>

namespace eq
{
namespace fabric
{
namespace detail
{
class ConfigParams
{
public:
    ConfigParams()
        : flags( eq::fabric::Global::getFlags( ))
        , prefixes( eq::fabric::Global::getPrefixes( ))
    {
        switch( flags & fabric::ConfigParams::FLAG_LOAD_EQ_ALL )
        {
            case fabric::ConfigParams::FLAG_LOAD_EQ_2D:
                equalizer.setMode( fabric::Equalizer::MODE_2D );
                break;
            case fabric::ConfigParams::FLAG_LOAD_EQ_HORIZONTAL:
                equalizer.setMode( fabric::Equalizer::MODE_HORIZONTAL );
                break;
            case fabric::ConfigParams::FLAG_LOAD_EQ_VERTICAL:
                equalizer.setMode( fabric::Equalizer::MODE_VERTICAL );
                break;
            case fabric::ConfigParams::FLAG_NONE:
                break;
            default:
                LBUNIMPLEMENTED;
        }
    }

    std::string name;
    std::string renderClient;
    Strings renderClientArgs;
    Strings renderClientEnvPrefixes;
    std::string workDir;
    uint32_t flags;
    fabric::Equalizer equalizer;
    Strings prefixes;
    std::string gpuFilter;
};
}

ConfigParams::ConfigParams()
    : _impl( new detail::ConfigParams )
{
}

ConfigParams::ConfigParams( const ConfigParams& rhs )
    : _impl( new detail::ConfigParams( *rhs._impl ))
{
}

ConfigParams& ConfigParams::operator = ( const ConfigParams& rhs )
{
    if( this == &rhs )
        return *this;

    *_impl = *rhs._impl;
    return *this;
}

ConfigParams::~ConfigParams()
{
    delete _impl;
}

void ConfigParams::setName( const std::string& name )
{
    _impl->name = name;
}

const std::string& ConfigParams::getName() const
{
    return _impl->name;
}

void ConfigParams::setRenderClient( const std::string& renderClient )
{
    _impl->renderClient = renderClient;
#ifdef _WIN32 // replace dir delimiters since '\' is often used as escape char
    std::replace( _impl->renderClient.begin(),
                  _impl->renderClient.end(), '\\', '/' );
#endif
}

const std::string& ConfigParams::getRenderClient() const
{
    return _impl->renderClient;
}

void ConfigParams::setRenderClientArgs( const Strings& args )
{
    _impl->renderClientArgs = args;
}

const Strings& ConfigParams::getRenderClientArgs() const
{
    return _impl->renderClientArgs;
}

void ConfigParams::setRenderClientEnvPrefixes( const Strings& prefixes )
{
    _impl->renderClientEnvPrefixes = prefixes;
}

const Strings& ConfigParams::getRenderClientEnvPrefixes() const
{
    return _impl->renderClientEnvPrefixes;
}

void ConfigParams::setWorkDir( const std::string& workDir )
{
    _impl->workDir = workDir;
#ifdef _WIN32 // replace dir delimiters since '\' is often used as escape char
    std::replace( _impl->workDir.begin(), _impl->workDir.end(), '\\', '/' );
#endif
}

const std::string& ConfigParams::getWorkDir() const
{
    return _impl->workDir;
}

void ConfigParams::setFlags( const uint32_t flags )
{
    _impl->flags = flags;
}

uint32_t ConfigParams::getFlags() const
{
    return _impl->flags;
}

const Equalizer& ConfigParams::getEqualizer() const
{
    return _impl->equalizer;
}

Equalizer& ConfigParams::getEqualizer()
{
    return _impl->equalizer;
}

void ConfigParams::setPrefixes( const Strings& prefixes )
{
    _impl->prefixes = prefixes;
}

const Strings& ConfigParams::getPrefixes() const
{
    return _impl->prefixes;
}

void ConfigParams::setGPUFilter( const std::string& regex )
{
    _impl->gpuFilter = regex;
}

const std::string& ConfigParams::getGPUFilter() const
{
    return _impl->gpuFilter;
}

void ConfigParams::serialize( co::DataOStream& os ) const
{
    os << _impl->name << _impl->renderClient << _impl->renderClientArgs
       << _impl->renderClientEnvPrefixes << _impl->workDir << _impl->flags
       << _impl->equalizer << _impl->prefixes << _impl->gpuFilter;
}

void ConfigParams::deserialize( co::DataIStream& is )
{
    is >> _impl->name >> _impl->renderClient >> _impl->renderClientArgs
       >> _impl->renderClientEnvPrefixes >> _impl->workDir >> _impl->flags
       >> _impl->equalizer >> _impl->prefixes >> _impl->gpuFilter;
}

co::DataOStream& operator << ( co::DataOStream& os, const ConfigParams& params )
{
    params.serialize( os );
    return os;
}

co::DataIStream& operator >> ( co::DataIStream& is, ConfigParams& params )
{
    params.deserialize( is );
    return is;
}

}
}

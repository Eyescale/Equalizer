
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "dso.h"

#include "debug.h"
#include "log.h"
#include "os.h"

#ifdef _WIN32 //_MSC_VER
#  define EQ_DL_ERROR sysError
#else
#  include <dlfcn.h>
#  define EQ_DL_ERROR dlerror()
#endif

namespace co
{
namespace base
{
namespace detail
{
class DSO
{
public:
    DSO() : dso( 0 ) {}

#ifdef _WIN32 //_MSC_VER
    HMODULE dso;
#else
    void* dso;
#endif
};
}

DSO::DSO()
        : _impl( new detail::DSO )
{}

DSO::~DSO()
{
    delete _impl;
}

bool DSO::open( const std::string& fileName )
{
    if( _impl->dso )
    {
        EQWARN << "DSO already open, close it first" << std::endl;
        return false;
    }

    if( fileName.empty( ))
    {
#ifdef _WIN32 //_MSC_VER
        _impl->dso = GetModuleHandle( 0 );
        EQASSERT( _impl->dso );
#else
        _impl->dso = RTLD_DEFAULT;
#endif
    }
    else
    {
#ifdef _WIN32 //_MSC_VER
        _impl->dso = LoadLibrary( fileName.c_str() );
#elif defined( RTLD_LOCAL )
        _impl->dso = dlopen( fileName.c_str(), RTLD_LAZY | RTLD_LOCAL );
#else
        _impl->dso = dlopen( fileName.c_str(), RTLD_LAZY );
#endif
        if( !_impl->dso )
        {
            EQINFO << "Can't open library: " << EQ_DL_ERROR << std::endl;
            return false;
        }
    }

    return true;
}

void DSO::close()
{
    if( !_impl->dso )
        return;

#ifdef _WIN32 //_MSC_VER
    if( _impl->dso != GetModuleHandle( 0 ))
        FreeLibrary( _impl->dso ) ;
#else
    if( _impl->dso != RTLD_DEFAULT )
        dlclose ( _impl->dso );
#endif

    _impl->dso = 0;
}

void* DSO::getFunctionPointer( const std::string& name )
{
#ifdef _WIN32 //_MSC_VER
    return (void*)GetProcAddress( _impl->dso, name.c_str() );
#else
    return dlsym( _impl->dso, name.c_str() );
#endif
}

bool DSO::isOpen() const
{
    return _impl->dso != 0;
}

}
}

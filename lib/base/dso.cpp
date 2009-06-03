/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
#include "log.h"

#ifdef WIN32
#  define EQ_DL_ERROR getErrorString( GetLastError( ))
#else
#  include <dlfcn.h>
#  define EQ_DL_ERROR dlerror()
#endif

namespace eq
{
namespace base
{

bool DSO::open( const std::string& fileName )
{
    if( _dso )
    {
        EQWARN << "DSO already open, close it first" << std::endl;
        return false;
    }

#ifdef WIN32
    _dso = LoadLibrary( fileName.c_str() );
#else
    _dso = dlopen( fileName.c_str(), RTLD_LAZY );
#endif

    if( !_dso )
    {
        EQWARN << "Can't open library: " << EQ_DL_ERROR << std::endl;
        return false;
    }
    return true;
}

void DSO::close()
{
    if( !_dso )
        return;

#ifdef WIN32
     FreeLibrary( _dso ) ;
#else
     dlclose ( _dso );
#endif

    _dso = 0;
}

void* DSO::getFunctionPointer( const std::string& name )
{
    if( !_dso )
    {
        EQWARN << "Attempting to retrieve function pointer from closed DSO"
               << std::endl;
        return 0;
    }

#ifdef WIN32
    return GetProcAddress( _dso, name.c_str() );
#else
    return dlsym( _dso, name.c_str() );
#endif
}

}
}

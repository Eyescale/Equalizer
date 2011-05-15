
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "application.h"
#include "detail/application.h"

#include <eq/config.h>
#include <eq/configParams.h>
#include <eq/init.h>
#include <eq/server.h>

namespace seq
{

Application::Application()
{
    _impl = new detail::Application( this );
}

Application::~Application()
{
    delete _impl;
    _impl = 0;
}

bool Application::init( const int argc, char** argv )
{
    if( _impl->isInitialized( ))
    {
        EQERROR << "Already initialized" << std::endl;
        return false;
    }

    if( !eq::init( argc, argv, _impl ))
    {
        EQERROR << "Equalizer initialization failed" << std::endl;
        return false;
    }

    if( !initLocal( argc, argv ))
    {
        EQERROR << "Can't initialization client node" << std::endl;
        exit();
        return false;
    }

    if( !_impl->init( ))
    {
        exit();
        return false;
    }
        
    return true;
}

bool Application::run()
{
    return true;
}

bool Application::exit()
{
    bool retVal = _impl->exit();
    if( !exitLocal( ))
        retVal = false;

    EQASSERTINFO( getRefCount() == 1, *this );
    if( !eq::exit( ))
        retVal = false;

    return retVal;
}

}

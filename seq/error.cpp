
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include "error.h"

#include <eq/fabric/errorRegistry.h>
#include <eq/fabric/global.h>

namespace seq
{

namespace
{
struct ErrorData
{
    const uint32_t code;
    const std::string text;
};

ErrorData _errors[] = {
    { ERROR_SEQUEL_MAPOBJECT_FAILED, 
      "Mapping of data from application process failed" },
    { ERROR_SEQUEL_CREATERENDERER_FAILED,
      "Application::newRenderer returned 0" },

    { 0, "" } // last!
};
}

void initErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::fabric::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.setString( _errors[i].code, _errors[i].text );
}

void exitErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::fabric::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.eraseString( _errors[i].code );
}

}


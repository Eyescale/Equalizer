
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "global.h"
#include "nodeFactory.h"
#include <co/base/os.h>
#include <co/base/lock.h>

namespace eq
{
NodeFactory* Global::_nodeFactory = 0;

#ifdef _WIN32 
# ifdef EQ_BUILD_DIR
   std::string Global::_configFile = "../../../../examples/configs/4-window.all.eqc";
# else
   std::string Global::_configFile = "../examples/configs/4-window.all.eqc";
# endif
#endif

#ifdef Darwin
    std::string Global::_configFile = "examples/configs/4-window.all.eqc";
#endif
#ifdef Linux
   std::string Global::_configFile = "examples/configs/4-window.all.eqc";
#endif

#ifdef AGL
static co::base::Lock _carbonLock;
#endif

void Global::setConfigFile( const std::string& configFile )
{
    _configFile = configFile;
}

const std::string& Global::getConfigFile()
{
    return _configFile;
}

void Global::enterCarbon()
{
#ifdef AGL
    _carbonLock.set();
#endif
}

void Global::leaveCarbon()
{ 
#ifdef AGL
    _carbonLock.unset();
#endif
}

}

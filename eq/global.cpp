
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "global.h"

#include "nodeFactory.h"
#ifdef EQUALIZER_USE_HWSD
#  include <hwsd/nodeInfo.h>
#endif
#include <lunchbox/lock.h>

namespace eq
{
std::string _programName;
std::string _workDir;
NodeFactory* Global::_nodeFactory = 0;

#ifdef EQUALIZER_USE_HWSD
std::string Global::_config = hwsd::NodeInfo::getLocalSession();
#else
std::string Global::_config = "configs/config.eqc";
#endif

#ifdef AGL
static lunchbox::Lock _carbonLock;
#endif

void Global::setProgramName( const std::string& programName )
{
    _programName = programName;
}

const std::string& Global::getProgramName()
{
    return _programName;
}

void Global::setWorkDir( const std::string& workDir )
{
    _workDir = workDir;
}

const std::string& Global::getWorkDir()
{
    return _workDir;
}

void Global::setConfig( const std::string& config )
{
    _config = config;
}

const std::string& Global::getConfig()
{
    return _config;
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

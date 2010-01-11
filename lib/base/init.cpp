
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "init.h"

#include "log.h"
#include "thread.h"
#include "global.h"
#include "pluginRegistry.h"

namespace eq
{
namespace base
{

EQ_EXPORT bool init()
{
    
    // init all available plugins
    PluginRegistry& pluginRegistry = Global::getPluginRegistry();
    pluginRegistry.init(); 
    Thread::pinCurrentThread();
    return true;
}

EQ_EXPORT bool exit()
{
    // de-initialize registered plugins
    PluginRegistry& pluginRegistry = Global::getPluginRegistry();
    pluginRegistry.exit(); 
    eq::base::Thread::removeAllListeners();
    eq::base::Log::exit();
    return true;
}

}
}



/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.h> 
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

#ifndef EQSERVER_CONFIG_RESOURCES_H
#define EQSERVER_CONFIG_RESOURCES_H

#include "../types.h"

namespace eq
{
namespace server
{
namespace config
{

class Resources
{
public:
    static bool discoverLocal( Config* config );
    static void configure( const Compounds& compounds );

private:
    static Compound* _add2DCompound( Compound* root, const Channels& channels );
    static Compound* _addEyeCompound( Compound* root, const Channels& channels);
};

}
}
}
#endif // EQSERVER_CONFIG_RESOURCES_H

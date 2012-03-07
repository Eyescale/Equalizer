
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.h> 
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

#define EQ_SERVER_CONFIG_LAYOUT_SIMPLE      "Simple"
#define EQ_SERVER_CONFIG_LAYOUT_2D_STATIC   "Static2D"
#define EQ_SERVER_CONFIG_LAYOUT_2D_DYNAMIC  "Dynamic2D"
#define EQ_SERVER_CONFIG_LAYOUT_DB_STATIC   "StaticDB"
#define EQ_SERVER_CONFIG_LAYOUT_DB_DYNAMIC  "DynamicDB"
#define EQ_SERVER_CONFIG_LAYOUT_DB_DS       "DBDirectSend"
#define EQ_SERVER_CONFIG_LAYOUT_DB_2D       "DB_2D"

namespace eq
{
namespace server
{
namespace config
{

class Resources
{
public:
    static bool discover( Config* config, const std::string& session,
                          const uint32_t flags );
    static Channels configureSourceChannels( Config* config );
    static void configure( const Compounds& compounds, const Channels& channels,
                           const uint32_t flags );

private:
    static Compound* _addMonoCompound( Compound* root, const Channels& channels,
                                       const uint32_t flags );
    static Compound* _addStereoCompound(Compound* root, const Channels& channels,
                                        const uint32_t flags );
    static Compound* _add2DCompound( Compound* root, const Channels& channels );
    static Compound* _addDBCompound( Compound* root, const Channels& channels );
    static Compound* _addDSCompound( Compound* root, const Channels& channels );
    static Compound* _addDB2DCompound( Compound* root,
                                       const Channels& channels );
    static const Compounds& _addSources( Compound* compound, const Channels& );
    static void _fill2DCompound( Compound* compound, const Channels& channels );
};

}
}
}
#endif // EQSERVER_CONFIG_RESOURCES_H

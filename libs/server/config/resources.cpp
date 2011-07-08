
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

#include "resources.h"

#include "../node.h"
#include "../pipe.h"

#include <eq/uiFactory.h>
#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace server
{
namespace config
{

void Resources::discoverLocal( Config* config )
{
    const GPUInfos infos = UIFactory::discoverGPUs();
    EQASSERT( !infos.empty( ));

    Node* node = new Node( config );
    node->setApplicationNode( true );

    for( GPUInfosCIter i = infos.begin(); i != infos.end(); ++i )
    {
        const GPUInfo& info = *i;

        Pipe* pipe = new Pipe( node );
        pipe->setPort( info.port );
        pipe->setDevice( info.device );
        pipe->setPixelViewport( info.pvp );
    }
}

}
}
}


/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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


#include <eq/admin.h>

namespace eqAdmin
{

inline bool addWindow( eq::admin::ServerPtr server )
{
    // Find first pipe...
    const eq::admin::ConfigVector& configs = server->getConfigs();
    if( configs.empty( ))
    {
        std::cout << "No configs on server, exiting" << std::endl;
        return false;
    }

    eq::admin::Config* config = configs.front();
    const eq::admin::NodeVector& nodes = config->getNodes();
    if( nodes.empty( ))
    {
        std::cout << "No nodes in config, exiting" << std::endl;
        return false;
    }
 
    const eq::admin::Node* node = nodes.front();
    const eq::admin::PipeVector& pipes = node->getPipes();
    if( pipes.empty( ))
    {
        std::cout << "No pipes in node, exiting" << std::endl;
        return false;
    }

    eq::admin::Pipe* pipe = pipes.front();
    EQASSERT( pipe );
    //std::cout << "Using " << *pipe << std::endl;

    // Add window (->channel->segment->canvas->layout->view)
    eq::admin::Window* window = new eq::admin::Window( pipe );
    eq::admin::Channel* channel = new eq::admin::Channel( window );
    eq::admin::Canvas* canvas = new eq::admin::Canvas( config );
    eq::admin::Segment* segment = new eq::admin::Segment( canvas );
    eq::admin::Layout* layout = new eq::admin::Layout( config );
    new eq::admin::View( layout );

    window->setViewport( eq::fabric::Viewport( 0.1f, 0.1f, 0.3f, 0.3f ));
    window->setName( "Runtime-created window" );
    
    segment->setChannel( channel );
    canvas->addLayout( layout );
    
    config->commit();
    return true;
}

}

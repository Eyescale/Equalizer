
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "eVolve.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <stdlib.h>

using namespace eq::base;
using namespace std;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig( eq::ServerPtr parent )
        { return new eVolve::Config( parent ); }
    virtual eq::Node*    createNode( eq::Config* parent )  
        { return new eVolve::Node( parent ); }
    virtual eq::Pipe*    createPipe( eq::Node* parent )
        { return new eVolve::Pipe( parent ); }
    virtual eq::Window*  createWindow( eq::Pipe* parent )
        { return new eVolve::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eVolve::Channel( parent ); }
};

int main( const int argc, char** argv )
{
    // 1. parse arguments
    eVolve::LocalInitData initData;
    initData.parseArguments( argc, argv );

    // 2. Equalizer initialization
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }
    
    // 3. initialization of local client node
    RefPtr< eVolve::EVolve > client = new eVolve::EVolve( initData );
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 4. run client
    const int ret = client->run();

    // 5. cleanup and exit
    client->exitLocal();
    client = 0;

    eq::exit();
    return ret;
}

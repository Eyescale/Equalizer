
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifdef WIN32
#define EQ_IGNORE_GLEW 
#endif

#include "osgScaleViewer.h"
#include "initData.h"
#include "nodeFactory.h"

#include <eq/client/init.h>
#include <eq/client/server.h>

int main( const int argc, char** argv )
{
    // 1. parse arguments
    osgScaleViewer::InitData initData;
    if ( !initData.parseCommandLine( argv, argc ))
        return -1;

    // 2. Equalizer initialization
    osgScaleViewer::NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        std::cout << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }

    // 3. initialization of local client node
    eq::base::RefPtr< osgScaleViewer::OSGScaleViewer > client =
        new osgScaleViewer::OSGScaleViewer( initData );
    if( !client->initLocal( argc, argv ))
    {
        std::cout << "Can't init client" << std::endl;
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

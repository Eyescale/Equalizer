
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include "osgScaleViewer.h"
#include "initData.h"
#include "nodeFactory.h"

int main( const int argc, char** argv )
{
    // 1. Equalizer initialization
    osgScaleViewer::NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        std::cout << "Equalizer init failed" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 2. parse arguments
    osgScaleViewer::InitData initData;
    if( !initData.parseCommandLine( argv, argc ))
        return -1;

    // 3. initialization of local client node
    lunchbox::RefPtr< osgScaleViewer::OSGScaleViewer > client =
        new osgScaleViewer::OSGScaleViewer( initData );
    if( !client->initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 4. run client
    const int ret = client->run();

    // 5. cleanup and exit
    client->exitLocal();

    LBASSERTINFO( client->getRefCount() == 1, "Client still referenced by " <<
                  client->getRefCount() - 1 );
    client = 0;

    eq::exit();
    return ret;
}

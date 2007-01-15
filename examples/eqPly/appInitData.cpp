
/*
 * Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 */

#include "appInitData.h"

#include <getopt.h>

void AppInitData::parseArguments( int argc, char** argv )
{
    int      result;
    int      index;
    struct option options[] = 
        {
            { "model",          required_argument, 0, 'm' },
            { "port",           required_argument, 0, 'p' },
            { 0,                0,                 0,  0 }
        };

    while( (result = getopt_long( argc, argv, "", options, &index )) != -1 )
    {
        switch( result )
        {
            case 'm':
                setFilename( optarg );
                break;

            case 'p':
                _trackerPort = optarg;

            default:
                EQWARN << "unhandled option: " << options[index].name << endl;
                break;
        }
    }
}

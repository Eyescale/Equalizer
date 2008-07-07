
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configParams.h"

#include <eq/net/global.h>

using namespace std;

namespace eq
{
ConfigParams::ConfigParams()
{
    _renderClient = eq::net::Global::getProgramName();
    _workDir      = eq::net::Global::getWorkDir();

//     compoundModes = 
//         COMPOUND_MODE_2D    | 
//         COMPOUND_MODE_DPLEX |
//         COMPOUND_MODE_EYE   |
//         COMPOUND_MODE_FSAA;
}

ConfigParams& ConfigParams::operator = ( const ConfigParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    _renderClient  = rhs._renderClient;
    _workDir       = rhs._workDir;
//    compoundModes = rhs.compoundModes;
    return *this;
}


void ConfigParams::setRenderClient( const std::string& renderClient )
{
    _renderClient = renderClient;
}

const std::string& ConfigParams::getRenderClient() const
{
    return _renderClient;
}

void ConfigParams::setWorkDir( const std::string& workDir )
{
    _workDir = workDir;
}

const std::string& ConfigParams::getWorkDir() const
{
    return _workDir;
}

}

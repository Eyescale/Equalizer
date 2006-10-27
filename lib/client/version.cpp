
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "version.h"
#include <sstream>

using namespace eq;

uint32_t Version::getMajor() 
{
    return EQ_VERSION_MAJOR; 
}
uint32_t Version::getMinor()
{
    return EQ_VERSION_MINOR; 
}
uint32_t Version::getPatch() 
{
    return EQ_VERSION_PATCH; 
}

uint32_t Version::getInt()
{
    return ( EQ_VERSION_MAJOR * 10000 +
             EQ_VERSION_MINOR * 100   +
             EQ_VERSION_PATCH ); 
}
float Version::getFloat() 
{
    return ( EQ_VERSION_MAJOR +
             .01f   * EQ_VERSION_MINOR   +
             .0001f * EQ_VERSION_PATCH ); 
}
std::string Version::getString()
{
    std::ostringstream  version;
    version << EQ_VERSION_MAJOR << '.' << EQ_VERSION_MINOR << '.'
            << EQ_VERSION_PATCH;
    return version.str();
}

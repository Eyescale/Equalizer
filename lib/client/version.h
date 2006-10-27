
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VERSION_H
#define EQ_VERSION_H

#include <sstream>

namespace eq
{
    // Equalizer version macros and functions
#   define EQ_VERSION_MAJOR 0
#   define EQ_VERSION_MINOR 1
#   define EQ_VERSION_PATCH 0

    class Version
    {
    public:
        static uint32_t getMajor() { return EQ_VERSION_MAJOR; }
        static uint32_t getMinor() { return EQ_VERSION_MINOR; }
        static uint32_t getPatch() { return EQ_VERSION_PATCH; }

        static uint32_t getInt()   { return ( EQ_VERSION_MAJOR * 10000 +
                                              EQ_VERSION_MINOR * 100   +
                                              EQ_VERSION_PATCH ); }
        static float    getFloat() { return ( EQ_VERSION_MAJOR +
                                              .01f   * EQ_VERSION_MINOR   +
                                              .0001f * EQ_VERSION_PATCH ); }
        static std::string getString()
            {
                std::ostringstream  version;
                version << EQ_VERSION_MAJOR << '.' << EQ_VERSION_MINOR << '.'
                        << EQ_VERSION_PATCH;
                return version.str();
            }
    };
}

#endif //EQ_VERSION_H

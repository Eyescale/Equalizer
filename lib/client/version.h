
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VERSION_H
#define EQ_VERSION_H

#include <eq/base/base.h>

#include <string>

namespace eq
{
    // Equalizer version macros and functions
#   define EQ_VERSION_MAJOR 0
#   define EQ_VERSION_MINOR 3
#   define EQ_VERSION_PATCH 9

    class Version
    {
    public:
        static uint32_t getMajor();
        static uint32_t getMinor();
        static uint32_t getPatch();

        static uint32_t getInt();
        static float    getFloat();
        static std::string getString();
    };
}

#endif //EQ_VERSION_H

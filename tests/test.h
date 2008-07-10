
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQTEST_TEST_H
#define EQTEST_TEST_H

#include <eq/base/log.h>

#define TEST( x )                                                       \
    if( !(x) )                                                          \
    {                                                                   \
        std::cerr << #x << " failed (l." << __LINE__ << ')' << std::endl; \
        ::exit( EXIT_FAILURE );                                         \
    }

#define TESTINFO( x, info )                                           \
    if( !(x) )                                                        \
    {                                                                 \
        std::cerr << #x << " failed (l." << __LINE__ << "): " << info \
                  << std::endl;                                       \
        ::exit( EXIT_FAILURE );                                       \
    }

#endif // EQTEST_TEST_H


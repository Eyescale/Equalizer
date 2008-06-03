
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQTEST_TEST_H
#define EQTEST_TEST_H

#include <eq/base/log.h>

#define TEST( x )                                 \
    if( !(x) )                                    \
    {                                             \
        cerr << #x << " failed." << std::endl;    \
        ::exit( EXIT_FAILURE );                   \
    }

#define TESTINFO( x, info )                                 \
    if( !(x) )                                              \
    {                                                       \
        cerr << #x << " failed: " << info << std::endl;     \
        ::exit( EXIT_FAILURE );                             \
    }

#endif // EQTEST_TEST_H


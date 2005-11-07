
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQTEST_TEST_H
#define EQTEST_TEST_H

#include <eq/base/log.h>

#define TEST( x ) \
    if( !(x) ) \
    { \
        ERROR << #x << " failed." << std::endl; \
        eqBase::dumpStack( std::cerr );         \
        ::abort();                              \
    }

#endif // EQTEST_TEST_H


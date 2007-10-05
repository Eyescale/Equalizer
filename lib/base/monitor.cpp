/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "monitor.h"


namespace eqBase
{
// instantiate base types
template class Monitor< uint32_t >;
template class Monitor< bool >;
}

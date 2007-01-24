
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifdef WIN32
#  include "pipeConnectionWin32.cpp"
#else
#  include "pipeConnectionPosix.cpp"
#endif

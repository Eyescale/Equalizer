/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// The cygwin flex insist on including unistd.h before any user files, which
// causes conflicts with winsock2.h included in Equalizer. This hack makes sure
// that winsock2.h is included first.

#include <eq/base/base.h>
#include "lexer.cpp"

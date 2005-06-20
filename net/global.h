
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_GLOBAL_H
#define EQNET_GLOBAL_H

/** 
 * @namespace eqNet
 * @brief The equalizer networking abstraction layer.
 *
 * The Equalizer network abstraction layer provides the basic functionality to
 * enable execution on distributed and shared memory machines. The access to the
 * actual C++ objects is deliberately hidden to encourage the use of identifiers
 * for portability to clusters. If this proves to be too restrictive, public
 * non-static member functions can be created later.
 */
namespace eqNet
{
    // global defines

    /** An invalid identifier. */
#   define INVALID_ID 0

    /** The any node identifier. */
#   define NODE_ID_ANY 0xffffffff

    /** The any type identifier. */
#   define TYPE_ID_ANY 0xffffffff
};

#endif // EQNET_GLOBAL_H



/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_INIT_H
#define EQNET_INIT_H

/** 
 * @namespace eqNet
 * @brief The Equalizer networking abstraction layer.
 *
 * The Equalizer network abstraction layer provides the basic functionality to
 * enable execution on distributed and shared memory machines.
 */
namespace eqNet
{
    class Node;

    /** 
     * Initialises the Equalizer client library.
     * 
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @return <code>true</code> if the library was successfully initialised,
     *         <code>false</code> otherwise.
     */
    bool init( int argc, char** argv );

    /**
     * De-initialises the Equalizer client library.
     *
     * @return <code>true</code> if the library was successfully de-initialised,
     *         <code>false</code> otherwise.
     */
    bool exit();
}

#endif // EQNET_INIT_H



/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_INIT_H
#define EQ_INIT_H

#include <eq/base/base.h>

/** 
 * @namespace eq
 * @brief The Equalizer client library.
 *
 * This namespace implements the application-visible API to access the Equalizer
 * server.
 */
namespace eq
{
	class NodeFactory;

    /** 
     * Initialises the Equalizer client library.
     *
     * This function may not return. This is used for remote render client nodes
     * which execute incoming commands from the server.
     * 
     * @param argc the command line argument count.
     * @param argv the command line argument values.
	 * @param nodeFactory the factory for allocating Equalizer objects.
	 *
     * @return <code>true</code> if the library was successfully initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool init( int argc, char** argv, NodeFactory* nodeFactory = 0 );
    
    /**
     * De-initialises the Equalizer client library.
     *
     * @return <code>true</code> if the library was successfully de-initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool exit();
}

#endif // EQNET_INIT_H


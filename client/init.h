
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_INIT_H
#define EQ_INIT_H

/** 
 * @namespace eq
 * @brief The Equalizer client library.
 *
 * This namespace implements the application-visible API to access the Equalizer
 * server.
 */
namespace eq
{
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


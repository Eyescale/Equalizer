
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_GLOBAL_H
#define EQNET_GLOBAL_H

/** 
 * @namespace eqNet
 * @brief The Equalizer networking abstraction layer.
 *
 * The Equalizer network abstraction layer provides the basic functionality to
 * enable execution on distributed and shared memory machines. The access to the
 * actual C++ objects is deliberately hidden to encourage the use of
 * identifiers. Concrete objects of the network may be deleted at any time by
 * another node, therefore pointers can become invalid. If this proves to be too
 * restrictive, public non-static member functions can be created later.
 */
namespace eqNet
{
    // global defines

    /** An invalid identifier. */
#   define INVALID_ID 0xfffffffe

    /** The node identifier of the server. */
#   define NODE_ID_SERVER 1

    /** The any node identifier. */
#   define NODE_ID_ANY 0xffffffff

    /** The any type identifier. */
#   define TYPE_ID_ANY 0xffffffff

    /** 
     * Initialises the Equalizer networking layer.
     * 
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     */
    void init( int argc, char** argv );

    class Global
    {
    public:
        static void        setProgramName( const char* programName );
        static const char* getProgramName();

    private:
        static char* _programName;
    };
}

#endif // EQNET_GLOBAL_H


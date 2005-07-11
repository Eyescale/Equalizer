
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SERVER_H
#define EQNET_SERVER_H

namespace eqNet
{
    class Session;

    /**
     * A Server is the central instance running multiple sessions.
     *
     * @sa Session
     */
    class Server
    {
    public:
        /**
         * @name Creating a server
         */
        //*{
        /** 
         * Runs the standalone server on the specified address.
         * 
         * @param address the address, if <code>NULL</code> the server will
         *                listen on all addresses of the machine using the
         *                default port.
         * @return the success value of the run.
         */
        static int run( const char* address );
        //*}
    };
}

#endif //EQNET_SERVER_H

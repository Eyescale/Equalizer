
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

namespace eqNet
{
    /**
     * Manages a session.
     *
     * A session represents a group of nodes managed by a central server. The
     * server ensures that all identifiers used during communication are
     * unique. 
     */
    class Session
    {
    public:
        /**
         * Initializes the network by connecting to an Equalizer server.
         *
         * @param server the server location.
         * @return the session.
         * @throws ??? if the server could not be contacted.
         */
        static Session* init( const char *server );
        
        /**
         * Joins an existing session on a server.
         *
         * @param server the server location.
         * @param session the session id.
         * @return the session.
         * @throws ??? if the server could not be contacted.
         */
        static Session* join( const char *server, const uint sessionID );

        /** @return the session id. */
        uint getID();

        /**
         * Gets the pointer to an existing session object.
         *
         * The session has to be created using Session::init() or Session::join
         * before using this function. It is a convenience function to enable
         * the usage of identifiers instead of pointers throughout the
         * application.
         *
         * @return the session.
         * @throws invalid_argument if the id is not known.
         */
        static Session* getSession( const uint id );

        /**
         * 
         */
        uint addNode( ProtocolDescription *desc );
// get, remove node
// add, get, remove protocol per node
void               enableForwarding( uint nid, Protocol p1, uint n1,
                                     Protocol p2, uint n2 );
void               disableForwarding( uint nid, Protocol p1, uint n1,
                                      Protocol p2, uint n2 );

bool   /*success*/ init();
void               exit();

bool   /*success*/ initNode( uint nid ); // late init may not be possible
void   /*success*/ exitNode( uint nid ); // early exit may not be possible

bool   /*success*/ startNode( uint nid );
void               stopNode( uint nid );

enum Protocol 
{
     PROTO_TCPIP,
     PROTO_MPI
};

ProtocolDescription 
{
    Protocol protocol;

    uint networkID = 0; // same protocol and network = direct connectivity
    uint64 bandwidthKBS;
    
    union 
    {
        struct TCPIP
        {
            const char *rshCommand; // e.g., "ssh eile@node1"
            const char *address;    // (<IP>|<name>)(:<port>)
        };
        struct MPI
        {
            const char *rshCommand; // e.g., "ssh eile@node1"
        };
    };
};

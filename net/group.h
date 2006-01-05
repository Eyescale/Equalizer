
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_GROUP_H
#define EQNET_GROUP_H

namespace eqNet
{
    /**
     * A Group is a collection of nodes for broadcast communications.
     *
     * Depending on the used network protocol, these communication methods may
     * used an optimised network transport mechanism.
     *
     * @sa Node
     */
    class Group
    {
    public:
        //void               broadCast( uint gid_to, Type type, void *ptr,
        //uint64_t count, uint flags );

        /** 
         * Performs a barrier operation with all members of this group.
         * 
         * @param groupID the identifier of the group.
         */
        static void barrier( const uint groupID );
    };
};

#endif //EQNET_GROUP_H

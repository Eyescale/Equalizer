
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_USER_PRIV_H
#define EQNET_USER_PRIV_H

#include "base.h"

#include <iostream>

namespace eqNet
{
    class User : public Base
    {
    public:
        /** 
         * Constructs a new user.
         * 
         * @param id the identifier of the user.
         */
        User(const uint id );

        /** 
         * Returns the identifier of this session.
         * 
         * @return the identifier.
         */
        uint getID() const { return _id; }

    private:
        uint _id;
    };

    inline std::ostream& operator << ( std::ostream& os, const User* user )
    {
        if( !user )
            os << "NULL user";
        else
            os << "    user " << user->getID() << "(" << (void*)user << ")";
        
        return os;
    }
}
#endif // EQNET_USER_PRIV_H


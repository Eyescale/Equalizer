
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PIPE_H
#define EQS_PIPE_H

#include <vector.h>

namespace eqs
{
    /**
     * The pipe.
     */
    class Pipe
    {
    public:
        /** 
         * Constructs a new Pipe.
         */
        Pipe();
    };

    inline std::ostream& operator << ( std::ostream& os, const Pipe* pipe)
    {
        if( !pipe )
        {
            os << "NULL pipe";
            return os;
        }

        os << "pipe " << (void*)pipe
        return os;
    }
};
#endif // EQS_PIPE_H

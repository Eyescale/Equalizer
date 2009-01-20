
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VISITORRESULT_H
#define EQ_VISITORRESULT_H

#include <iostream>

namespace eq
{
     /** The result code from any visit operation. */
    enum VisitorResult
    {
        TRAVERSE_CONTINUE,   //!< continue the traversal
        TRAVERSE_TERMINATE,  //!< abort the traversal
        TRAVERSE_PRUNE       //!< do not traverse current entity downwards
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const VisitorResult result )
    {
        switch( result )
        {
            case TRAVERSE_CONTINUE:
                os << "continue";
                break;
            case TRAVERSE_TERMINATE:
                os << "terminate";
                break;
            case TRAVERSE_PRUNE:
                os << "prune";
                break;
            default:
                os << "ERROR";
                break;
        }
        return os;
    }
}
#endif // EQ_VISITORRESULT_H


/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDACTIVATEVISITOR_H
#define EQSERVER_COMPOUNDACTIVATEVISITOR_H

#include "compoundVisitor.h" // base class

namespace eq
{
namespace server
{
    /** The compound visitor (de-)activating a compound tree. */
    class CompoundActivateVisitor : public CompoundVisitor
    {
    public:
        CompoundActivateVisitor( const bool activate ) : _activate( activate ){}
        virtual ~CompoundActivateVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound )
            {
                compound->setActive( _activate );
                
                Channel* channel = compound->getChannel();
                if( channel )
                {
                    if( _activate )
                        channel->activate();
                    else
                        channel->deactivate();
                }
                return TRAVERSE_CONTINUE;
            }

    private:
            const bool _activate;
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H

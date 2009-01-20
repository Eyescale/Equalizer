
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIGUPDATEDATAVISITOR_H
#define EQSERVER_CONFIGUPDATEDATAVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
    /**
     * The config visitor updating config data after the compound update
     */
    class ConfigUpdateDataVisitor : public ConfigVisitor
    {
    public:
        ConfigUpdateDataVisitor();
        virtual ~ConfigUpdateDataVisitor() {}

        virtual VisitorResult visitPre( Node* node );
        virtual VisitorResult visitPost( Node* node );
        virtual VisitorResult visitPre( Pipe* pipe );
        virtual VisitorResult visitPost( Pipe* pipe );
        virtual VisitorResult visitPre( Window* window );
        virtual VisitorResult visitPost( Window* window );
        virtual VisitorResult visit( Channel* channel );

        // No need to traverse compounds
        virtual VisitorResult visitPre( Compound* compound )
            { return TRAVERSE_PRUNE; }
 
    private:
        const Channel* _lastDrawChannel;
        const Window*  _lastDrawWindow;
        const Pipe*    _lastDrawPipe;
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

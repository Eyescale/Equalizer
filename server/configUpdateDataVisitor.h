
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

        virtual Result visitPre( Node* node );
        virtual Result visitPost( Node* node );
        virtual Result visitPre( Pipe* pipe );
        virtual Result visitPost( Pipe* pipe );
        virtual Result visitPre( Window* window );
        virtual Result visitPost( Window* window );
        virtual Result visit( Channel* channel );

        // No need to traverse compounds
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return Compound::TRAVERSE_PRUNE; }
 
    private:
        const Channel* _lastDrawChannel;
        const Window*  _lastDrawWindow;
        const Pipe*    _lastDrawPipe;
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H


/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_NAMEFINDER_H
#define EQSERVER_NAMEFINDER_H

#include "configVisitor.h"  // base class
#include "types.h"
 
namespace eq
{
namespace server
{

template< typename P, typename T > class NameFinder : public P
{
public:
    NameFinder( const std::string& name ) 
            : _name( name ), _result( 0 ) {}
    virtual ~NameFinder(){}

    virtual VisitorResult visitPre( T* node ) { return visit( node ); }
    virtual VisitorResult visit( T* node )
        {
            if( node->getName() == _name )
            {
                _result = node;
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const std::string _name;
    T*                _result;
};

typedef NameFinder< ConfigVisitor, Layout > LayoutFinder;
typedef NameFinder< ConstConfigVisitor, const Layout > ConstLayoutFinder;

typedef NameFinder< ConfigVisitor, View > ViewFinder;
typedef NameFinder< ConstConfigVisitor, const View > ConstViewFinder;

typedef NameFinder< ConfigVisitor, Canvas > CanvasFinder;
typedef NameFinder< ConstConfigVisitor, const Canvas > ConstCanvasFinder;

typedef NameFinder< ConfigVisitor, Segment > SegmentFinder;
typedef NameFinder< ConstConfigVisitor, const Segment > ConstSegmentFinder;

typedef NameFinder< ConfigVisitor, Channel > ChannelFinder;
typedef NameFinder< ConstConfigVisitor, const Channel > ConstChannelFinder;

}
}

#endif // EQSERVER_NAMEFINDER_H



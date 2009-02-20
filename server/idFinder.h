
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_IDFINDER_H
#define EQSERVER_IDFINDER_H

#include "configVisitor.h"  // base class
#include "types.h"
 
namespace eq
{
namespace server
{

template< typename P, typename T > class IDFinder : public P
{
public:
    IDFinder( const uint32_t id ) : _id( id ), _result( 0 ) {}
    virtual ~IDFinder(){}

    virtual VisitorResult visitPre( T* node ) { return visit( node ); }
    virtual VisitorResult visit( T* node )
        {
            if( node->getID() == _id )
            {
                _result = node;
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const uint32_t _id;
    T*             _result;
};

typedef IDFinder< ConfigVisitor, Layout > LayoutIDFinder;
typedef IDFinder< ConstConfigVisitor, const Layout > ConstLayoutIDFinder;

typedef IDFinder< ConfigVisitor, View > ViewIDFinder;
typedef IDFinder< ConstConfigVisitor, const View > ConstViewIDFinder;

typedef IDFinder< ConfigVisitor, Canvas > CanvasIDFinder;
typedef IDFinder< ConstConfigVisitor, const Canvas > ConstCanvasIDFinder;

typedef IDFinder< ConfigVisitor, Segment > SegmentIDFinder;
typedef IDFinder< ConstConfigVisitor, const Segment > ConstSegmentIDFinder;

typedef IDFinder< ConfigVisitor, Channel > ChannelIDFinder;
typedef IDFinder< ConstConfigVisitor, const Channel > ConstChannelIDFinder;

}
}

#endif // EQSERVER_IDFINDER_H



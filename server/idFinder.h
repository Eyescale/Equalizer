
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

typedef IDFinder< ConfigVisitor, Observer > ObserverIDFinder;
typedef IDFinder< ConstConfigVisitor, const Observer > ConstObserverIDFinder;

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




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

typedef NameFinder< ConfigVisitor, Observer > ObserverFinder;
typedef NameFinder< ConstConfigVisitor, const Observer > ConstObserverFinder;

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



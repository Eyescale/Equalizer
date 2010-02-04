
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

template< typename T > class IDFinder : public ConfigVisitor
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

typedef IDFinder< Observer > ObserverIDFinder;
typedef IDFinder< const Observer > ConstObserverIDFinder;

typedef IDFinder< Layout > LayoutIDFinder;
typedef IDFinder< const Layout > ConstLayoutIDFinder;

typedef IDFinder< View > ViewIDFinder;
typedef IDFinder< const View > ConstViewIDFinder;

typedef IDFinder< Canvas > CanvasIDFinder;
typedef IDFinder< const Canvas > ConstCanvasIDFinder;

typedef IDFinder< Segment > SegmentIDFinder;
typedef IDFinder< const Segment > ConstSegmentIDFinder;

typedef IDFinder< Channel > ChannelIDFinder;
typedef IDFinder< const Channel > ConstChannelIDFinder;

}
}

#endif // EQSERVER_IDFINDER_H



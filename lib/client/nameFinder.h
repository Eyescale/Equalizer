
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

#ifndef EQ_NAMEFINDER_H
#define EQ_NAMEFINDER_H

#include "configVisitor.h"  // base class
#include "types.h"
 
namespace eq
{

template< typename T > class NameFinder : public ConfigVisitor
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

typedef NameFinder< Observer > ObserverFinder;
typedef NameFinder< const Observer > ConstObserverFinder;

typedef NameFinder< Layout > LayoutFinder;
typedef NameFinder< const Layout > ConstLayoutFinder;

typedef NameFinder< View > ViewFinder;
typedef NameFinder< const View > ConstViewFinder;

typedef NameFinder< Canvas > CanvasFinder;
typedef NameFinder< const Canvas > ConstCanvasFinder;

typedef NameFinder< Segment > SegmentFinder;
typedef NameFinder< const Segment > ConstSegmentFinder;

typedef NameFinder< Channel > ChannelFinder;
typedef NameFinder< const Channel > ConstChannelFinder;

}

#endif // EQ_NAMEFINDER_H



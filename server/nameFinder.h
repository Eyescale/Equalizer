
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

#ifndef EQSERVER_NAMEFINDER_H
#define EQSERVER_NAMEFINDER_H

#include "configVisitor.h"  // base class
#include "types.h"

// TODO move to fabric namespace once class migration is done
 
namespace
{

template< typename T > class NameFinder : public eq::server::ConfigVisitor
{
public:
    NameFinder( const std::string& name ) 
            : _name( name ), _result( 0 ) {}
    virtual ~NameFinder(){}

    virtual eq::server::VisitorResult visitPre( T* node )
        { return visit( node ); }
    virtual eq::server::VisitorResult visit( T* node )
        {
            if( node->getName() == _name )
            {
                _result = node;
                return eq::server::TRAVERSE_TERMINATE;
            }
            return eq::server::TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const std::string _name;
    T*                _result;
};

typedef NameFinder< eq::server::Observer > ObserverFinder;
typedef NameFinder< const eq::server::Observer > ConstObserverFinder;

typedef NameFinder< eq::server::Layout > LayoutFinder;
typedef NameFinder< const eq::server::Layout > ConstLayoutFinder;

typedef NameFinder< eq::server::View > ViewFinder;
typedef NameFinder< const eq::server::View > ConstViewFinder;

typedef NameFinder< eq::server::Canvas > CanvasFinder;
typedef NameFinder< const eq::server::Canvas > ConstCanvasFinder;

typedef NameFinder< eq::server::Segment > SegmentFinder;
typedef NameFinder< const eq::server::Segment > ConstSegmentFinder;

typedef NameFinder< eq::server::Channel > ChannelFinder;
typedef NameFinder< const eq::server::Channel > ConstChannelFinder;

}

#endif // EQSERVER_NAMEFINDER_H



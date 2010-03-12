
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

// TODO move to fabric namespace once class migration is done
 
namespace
{

template< typename T > class IDFinder : public eq::server::ConfigVisitor
{
public:
    IDFinder( const uint32_t id ) : _id( id ), _result( 0 ) {}
    virtual ~IDFinder(){}

    virtual eq::server::VisitorResult visitPre( T* node )
        { return visit( node ); }
    virtual eq::server::VisitorResult visit( T* node )
        {
            if( node->getID() == _id )
            {
                _result = node;
                return eq::server::TRAVERSE_TERMINATE;
            }
            return eq::server::TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const uint32_t _id;
    T*             _result;
};

typedef IDFinder< eq::server::Observer > ObserverIDFinder;
typedef IDFinder< const eq::server::Observer > ConstObserverIDFinder;

typedef IDFinder< eq::server::Layout > LayoutIDFinder;
typedef IDFinder< const eq::server::Layout > ConstLayoutIDFinder;

typedef IDFinder< eq::server::View > ViewIDFinder;
typedef IDFinder< const eq::server::View > ConstViewIDFinder;

typedef IDFinder< eq::server::Canvas > CanvasIDFinder;
typedef IDFinder< const eq::server::Canvas > ConstCanvasIDFinder;

typedef IDFinder< eq::server::Segment > SegmentIDFinder;
typedef IDFinder< const eq::server::Segment > ConstSegmentIDFinder;

typedef IDFinder< eq::server::Channel > ChannelIDFinder;
typedef IDFinder< const eq::server::Channel > ConstChannelIDFinder;

}

#endif // EQSERVER_IDFINDER_H



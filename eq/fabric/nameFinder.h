
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_NAMEFINDER_H
#define EQFABRIC_NAMEFINDER_H

#include "types.h"

namespace eq
{
namespace fabric
{
template< class T, class V > class NameFinder : public V
{
public:
    explicit NameFinder( const std::string& name )
        : _name( name ), _result( 0 ) {}
    virtual ~NameFinder(){}

    virtual VisitorResult visitPre( T* entity ) { return visit( entity ); }
    virtual VisitorResult visit( T* entity )
    {
        if( entity->getName() == _name )
        {
            _result = entity;
            return TRAVERSE_TERMINATE;
        }
        return TRAVERSE_CONTINUE;
    }

    T* getResult() { return _result; }

private:
    const std::string _name;
    T*                _result;
};

}
}
#endif // EQFABRIC_NAMEFINDER_H

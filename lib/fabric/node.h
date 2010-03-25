
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_NODE_H
#define EQFABRIC_NODE_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/paths.h>
#include <eq/fabric/pixelViewport.h> // base class
#include <eq/fabric/types.h>

namespace eq
{

namespace fabric
{
    template< class C, class N, class P > class Node : public Object
    {
    public:
        typedef std::vector< P* > PipeVector;

        /** @name Data Access */
        //@{
        /** @return the index path to this node. @internal */
        EQFABRIC_EXPORT NodePath getPath() const;
        //@}

    protected:
        virtual ChangeType getChangeType() const { return UNBUFFERED; }
    };
}
}

#endif // EQFABRIC_NODE_H


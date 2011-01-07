
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CONFIGVISITOR_H
#define EQSERVER_CONFIGVISITOR_H

#include "compoundVisitor.h"           // base class
#include "types.h"

#include <eq/fabric/elementVisitor.h>  // base classes

namespace eq
{
namespace server
{
    class Config;

    /** A visitor to traverse configs and children. */
    class ConfigVisitor : public NodeVisitor, 
                          public CompoundVisitor,
                          public ObserverVisitor,
                          public LayoutVisitor,
                          public CanvasVisitor
    {
    public:
        /** Constructs a new ConfigVisitor. */
        ConfigVisitor(){}
        
        /** Destruct the ConfigVisitor */
        virtual ~ConfigVisitor(){}

        /** Visit a config on the down traversal. */
        virtual VisitorResult visitPre( Config* config )
            { return visitPre( static_cast< const Config* >( config )); }

        /** Visit a config on the up traversal. */
        virtual VisitorResult visitPost( Config* config )
            { return visitPost( static_cast< const Config* >( config )); }

        /** Visit a config on the down traversal. */
        virtual VisitorResult visitPre( const Config* config )
            { return TRAVERSE_CONTINUE; }

        /** Visit a config on the up traversal. */
        virtual VisitorResult visitPost( const Config* config )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CONFIGVISITOR_H

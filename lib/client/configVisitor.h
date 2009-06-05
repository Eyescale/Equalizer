
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CONFIGVISITOR_H
#define EQ_CONFIGVISITOR_H

#include <eq/client/canvasVisitor.h>        // base class
#include <eq/client/layoutVisitor.h>        // base class
#include <eq/client/nodeVisitor.h>          // base class
#include <eq/client/observerVisitor.h>          // base class

namespace eq
{
    class Config;

    /**
     * A visitor to traverse non-const configs and children.
     */
    class ConfigVisitor : public NodeVisitor, 
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
            { return TRAVERSE_CONTINUE; }

        /** Visit a config on the up traversal. */
        virtual VisitorResult visitPost( Config* config )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_CONFIGVISITOR_H

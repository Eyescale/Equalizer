
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

#ifndef EQFABRIC_CONFIGVISITOR_H
#define EQFABRIC_CONFIGVISITOR_H

#include <eq/fabric/types.h>
#include <eq/fabric/elementVisitor.h>       // base class

namespace eq
{
namespace fabric
{
    /** A visitor to traverse configs and all children. @sa Config::accept() */
    template< class C, class OV, class LV, class CV, class NV >
    class ConfigVisitor : public OV, public LV, public CV, public NV
    {
    public:
        /** Construct a new config visitor. @version 1.0 */
        ConfigVisitor(){}
        
        /** Destruct this config visitor. @version 1.0 */
        virtual ~ConfigVisitor(){}

        /** Visit a config on the down traversal. @version 1.0 */
        virtual VisitorResult visitPre( C* config )
            { return visitPre( static_cast< const C* >( config )); }

        /** Visit a config on the up traversal. @version 1.0 */
        virtual VisitorResult visitPost( C* config )
            { return visitPost( static_cast< const C* >( config )); }

        /** Visit a config on the down traversal. @version 1.0 */
        virtual VisitorResult visitPre( const C* )
            { return TRAVERSE_CONTINUE; }

        /** Visit a config on the up traversal. @version 1.0 */
        virtual VisitorResult visitPost( const C* )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQFABRIC_CONFIGVISITOR_H

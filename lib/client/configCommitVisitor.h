
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

#ifndef EQ_CONFIGCOMMITVISITOR_H
#define EQ_CONFIGCOMMITVISITOR_H

#include <eq/client/types.h>
#include <eq/fabric/configVisitor.h> // base class

namespace eq
{
    class Config;

    /** 
     * Commits all dirty config object at the beginning of each frame.
     * @internal
     */
    class ConfigCommitVisitor : public ConfigVisitor
    {
    public:
        ConfigCommitVisitor() {}
        virtual ~ConfigCommitVisitor() {}

        virtual VisitorResult visit( Observer* observer )
            { 
                observer->commit();
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                static_cast< net::Object*>( canvas )->commit();
                return TRAVERSE_PRUNE; // no need to visit segments
            }
        virtual VisitorResult visit( View* view )
            {
                static_cast< net::Object*>( view )->commit();
                return TRAVERSE_CONTINUE; 
            }
    };
}

#endif // EQ_CONFIGDESERIALIZER_H


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

#ifndef EQADMIN_CONFIGCOMMITVISITOR_H
#define EQADMIN_CONFIGCOMMITVISITOR_H

#include <eq/admin/pipe.h>
#include <eq/admin/segment.h>
#include <eq/admin/types.h>
#include <eq/admin/visitorResult.h>
#include <eq/admin/window.h>

#include <eq/fabric/configVisitor.h> // base class

namespace eq
{
namespace admin
{
    /** 
     * Commits all dirty config object at the beginning of each frame.
     * @internal
     */
    class ConfigCommitVisitor : public ConfigVisitor
    {
    public:
        ConfigCommitVisitor(){}
        virtual ~ConfigCommitVisitor() {}

        virtual VisitorResult visitPre( Pipe* pipe )
            {
                if( pipe->getID() != EQ_ID_INVALID )
                    pipe->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Window* window )
            {
                if( window->getID() != EQ_ID_INVALID )
                    window->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visit( Channel* channel )
            {
                if( channel->getID() != EQ_ID_INVALID )
                    channel->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Layout* layout )
            {
                if( layout->getID() != EQ_ID_INVALID )
                    layout->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visit( View* view )
            {
                if( view->getID() != EQ_ID_INVALID )
                    view->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Canvas* canvas )
            {
                if( canvas->getID() != EQ_ID_INVALID )
                    canvas->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visit( Segment* segment )
            {
                if( segment->getID() != EQ_ID_INVALID )
                    segment->commit();
                return TRAVERSE_CONTINUE;
            }
    };
}
}
#endif // EQADMIN_CONFIGDESERIALIZER_H

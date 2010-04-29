
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

#ifndef EQSERVER_CONFIGSYNCVISITOR_H
#define EQSERVER_CONFIGSYNCVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
    /** Synchronizes changes from the app at the beginning of each frame. */
    class ConfigSyncVisitor : public ConfigVisitor
    {
    public:
        ConfigSyncVisitor() {}
        virtual ~ConfigSyncVisitor() {}

        virtual VisitorResult visit( Observer* observer )
            {
                observer->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                canvas->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( View* view )
            {
                view->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Window* window )
            {
                window->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Pipe* pipe )
            {
                pipe->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Node* node )
            {
                node->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Channel* channel )
            {
                channel->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Compound* compound )
            {
                return TRAVERSE_TERMINATE;
            }
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

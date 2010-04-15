
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

#ifndef EQSERVER_CONFIGRESTOREVISITOR_H
#define EQSERVER_CONFIGRESTOREVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
namespace
{
    /** Restore all data which may have been modified by an application. */
    class ConfigRestoreVisitor : public ConfigVisitor
    {
    public:
        ConfigRestoreVisitor() {}
        virtual ~ConfigRestoreVisitor() {}

        virtual VisitorResult visitPre( Config* config )
            {
                config->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Observer* observer )
            {
                observer->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                canvas->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Segment* segment )
            {
                segment->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Layout* layout )
            {
                layout->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( View* view )
            {
                view->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Node* node )
            {
                node->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Pipe* pipe )
            {
                pipe->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Window* window )
            {
                window->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Channel* channel )
            {
                channel->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Compound* compound )
            {
                //compound->restore();
                return TRAVERSE_CONTINUE;
            }
    };
}
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

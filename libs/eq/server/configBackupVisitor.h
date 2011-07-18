
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_CONFIGBACKUPVISITOR_H
#define EQSERVER_CONFIGBACKUPVISITOR_H

#include "configVisitor.h" // base class

#include "canvas.h"
#include "compound.h"
#include "layout.h"
#include "observer.h"
#include "segment.h"
#include "view.h"

namespace eq
{
namespace server
{
namespace
{
    /** Backup all data which may be modified by a running application. */
    class ConfigBackupVisitor : public ConfigVisitor
    {
    public:
        ConfigBackupVisitor() {}
        virtual ~ConfigBackupVisitor() {}
        virtual VisitorResult visitPre( Compound* compound )
            {
                compound->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Config* config )
            {
                config->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Observer* observer )
            {
                observer->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                canvas->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Segment* segment )
            {
                segment->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Layout* layout )
            {
                layout->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( View* view )
            {
                view->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Node* node )
            {
                node->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Pipe* pipe )
            {
                pipe->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Window* window )
            {
                window->backup();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Channel* channel )
            {
                channel->backup();
                return TRAVERSE_CONTINUE;
            }
    };
}
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

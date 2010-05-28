
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

        virtual VisitorResult visitPre( Config* config )
            {
                config->sync( net::VERSION_HEAD );
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Node* node )
            {
                node->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Pipe* pipe )
            {
                pipe->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Window* window )
            {
                window->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Channel* channel )
            {
                channel->sync();
                View* view = channel->getView();
                if( view )
                    visit( view );
                channel->setViewVersion( view );
                channel->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Window* window )
            {
                window->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Pipe* pipe )
            {
                pipe->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Node* node )
            {
                node->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visit( Observer* observer )
            {
                observer->sync();
                observer->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Canvas* canvas )
            {
                canvas->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Segment* segment )
            {
                segment->sync();
                segment->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Canvas* canvas )
            {
                canvas->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Layout* layout )
            {
                layout->sync();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( View* view )
            {
                view->sync();

                // Don't call commit on non-dirty views:
                //  Commit auto-obsoletes old versions which breaks latency
                //  saving without this save-guard. We are called here from each
                //  channel using this view (to have the new version while
                //  commiting the channel) and during traversal.
                if( view->isDirty( ))
                    view->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Layout* layout )
            {
                layout->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Compound* compound )
            {
                return TRAVERSE_PRUNE;
            }

        virtual VisitorResult visitPost( Config* config )
            {
                config->commit();
                return TRAVERSE_CONTINUE;
            }
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

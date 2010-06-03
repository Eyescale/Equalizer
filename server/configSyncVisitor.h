
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
namespace
{
    /** Synchronizes view changes at the beginning of each frame. */
    class PreSyncVisitor : public ConfigVisitor
    {
    public:
        virtual ~PreSyncVisitor() {}

        // Optimize traversal
        virtual VisitorResult visitPre( Node* node ) { return TRAVERSE_PRUNE; }
        virtual VisitorResult visitPre( Canvas* canvas )
            { return TRAVERSE_PRUNE; }
        virtual VisitorResult visitPre( Compound* compound )
            { return TRAVERSE_TERMINATE; }

        virtual VisitorResult visit( Observer* observer )
            {
                observer->sync();
                observer->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visit( View* view )
            {
                view->sync();
                view->commit();
                return TRAVERSE_CONTINUE;
            }
    };
}

    /** Synchronizes changes from the app at the beginning of each frame. */
    class ConfigSyncVisitor : public ConfigVisitor
    {
    public:
        virtual ~ConfigSyncVisitor() {}

        virtual VisitorResult visitPre( Config* config )
            {
                config->sync( net::VERSION_HEAD );
                // Commit observers & views first, they are ref'ed be channels.
                PreSyncVisitor preSyncer;
                config->accept( preSyncer );
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
                channel->setViewVersion( channel->getView( ));
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

        virtual VisitorResult visitPre( Canvas* canvas )
            {
                return _sync( canvas );
            }
        virtual VisitorResult visit( Segment* segment )
            {
                return _update( segment );
            }
        virtual VisitorResult visitPost( Canvas* canvas )
            {
                if( canvas->getID() <= EQ_ID_MAX )
                    canvas->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Layout* layout )
            {
                layout->sync();
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

    private:
        template< class T > VisitorResult _sync( T* entity )
            {
                if( entity->getID() <= EQ_ID_MAX )
                    entity->sync();
                else
                {
                    EQASSERT( entity->needsDelete( ));
                }
                return TRAVERSE_CONTINUE;
            }

        template< class T > VisitorResult _update( T* entity )
            {
                if( entity->getID() <= EQ_ID_MAX )
                {
                    entity->sync();
                    entity->commit();
                }
                return TRAVERSE_CONTINUE;
            }
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H

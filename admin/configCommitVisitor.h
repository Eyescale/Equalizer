
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
#include <eq/fabric/packets.h>

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

        virtual VisitorResult visitPre( Window* window )
            {
                _register< Pipe, Window, fabric::PipeNewWindowPacket >( 
                    window->getPipe(), window );
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Channel* channel )
            {
                _register< Window, Channel, fabric::WindowNewChannelPacket >( 
                    channel->getWindow(), channel );
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

        virtual VisitorResult visit( Observer* observer )
            {
                _register< Config, Observer, fabric::ConfigNewObserverPacket >( 
                    observer->getConfig(), observer );
                observer->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Layout* layout )
            {
                _register< Config, Layout, fabric::ConfigNewLayoutPacket >( 
                    layout->getConfig(), layout );
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( View* view )
            {
                _register< Layout, View, fabric::LayoutNewViewPacket >( 
                    view->getLayout(), view );
                view->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Layout* layout )
            {
                layout->commit();
                return TRAVERSE_CONTINUE;
            }

        virtual VisitorResult visitPre( Canvas* canvas )
            {
                _register< Config, Canvas, fabric::ConfigNewCanvasPacket >( 
                    canvas->getConfig(), canvas );
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Segment* segment )
            {
                _register< Canvas, Segment, fabric::CanvasNewSegmentPacket >( 
                    segment->getCanvas(), segment );
                segment->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPost( Canvas* canvas )
            {
                canvas->commit();
                return TRAVERSE_CONTINUE;
            }

    private:
        template< class P, class E, class PKG > 
        void _register( P* parent, E* entity )
            {
                if( entity->getID() <= EQ_ID_MAX )
                    return;

                admin::Config* config = entity->getConfig();
                net::NodePtr localNode = config->getLocalNode();
                PKG packet;
                packet.requestID = localNode->registerRequest();

                net::NodePtr node = config->getServer().get();
                parent->send( node, packet );

                uint32_t identifier;
                localNode->waitRequest( packet.requestID, identifier );
                EQASSERT( identifier <= EQ_ID_MAX );
                EQCHECK( config->mapObject( entity, identifier,
                                            net::VERSION_NONE ));
            }

    };
}
}
#endif // EQADMIN_CONFIGDESERIALIZER_H

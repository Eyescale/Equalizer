
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
 *               2010, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "channel.h"

#include "node.h"

#include <eq/fabric/commands.h>


namespace eq
{
namespace server
{

class ChannelStopFrameVisitor : public ConfigVisitor
{
public:
    explicit ChannelStopFrameVisitor( const uint32_t lastFrameNumber )
        : _lastFrameNumber( lastFrameNumber ) {}

    virtual VisitorResult visit( Channel* channel )
    {
        if( !channel->isActive() || !channel->isRunning( ))
            return TRAVERSE_CONTINUE;

        channel->send( fabric::CMD_CHANNEL_STOP_FRAME ) << _lastFrameNumber;
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visitPost( Node* node )
    {
        if( node->isRunning() )
            node->flushSendBuffer();
        return TRAVERSE_CONTINUE;
    }

private:
    const uint32_t _lastFrameNumber;
};

}
}

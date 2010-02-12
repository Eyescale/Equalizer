
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

#ifndef EQ_CHANNELVISITOR_H
#define EQ_CHANNELVISITOR_H

#include <eq/client/visitorResult.h>  // enum

namespace eq
{
    class Channel;

    /** A visitor to traverse channels. @sa Channel::accept() */
    class ChannelVisitor
    {
    public:
        /** Constructs a new ChannelVisitor. @version 1.0 */
        ChannelVisitor(){}
        
        /** Destruct the ChannelVisitor @version 1.0 */
        virtual ~ChannelVisitor(){}

        /** Visit a channel. @version 1.0 */
        virtual VisitorResult visit( Channel* channel )
            { return visit( static_cast< const Channel* >( channel )); }

        /** Visit a channel during a const traversal. @version 1.0 */
        virtual VisitorResult visit( const Channel* channel )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_CHANNELVISITOR_H

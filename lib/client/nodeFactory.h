
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_NODEFACTORY_H
#define EQ_NODEFACTORY_H

#include <eq/client/types.h>
#include <eq/base/base.h>

namespace eq
{
    /**
     * The node factory is a per-node singleton used to create and release
     * Equalizer resource instances.
     *
     * The instances have to be subclasses of the corresponding Equalizer
     * classes, and are used to selectively override task methods and store
     * additional, application-specific data.
     */
    class EQ_EXPORT NodeFactory
    {
    public:
        /** 
         * Create a new config.
         * 
         * @return the config.
         */
        virtual Config* createConfig( ServerPtr parent );

        /** Release a config. */
        virtual void releaseConfig( Config* config );

        /** 
         * Create a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode( Config* parent );

        /** Release a node. */
        virtual void releaseNode( Node* node );

        /** 
         * Create a new observer.
         * 
         * @return the observer.
         */
        virtual Observer* createObserver( Config* parent );

        /** Release a observer. */
        virtual void releaseObserver( Observer* observer );

        /** 
         * Create a new layout.
         * 
         * @return the layout.
         */
        virtual Layout* createLayout( Config* parent );

        /** Release a layout. */
        virtual void releaseLayout( Layout* layout );

        /** 
         * Create a new view.
         * 
         * @return the view.
         */
        virtual View* createView( Layout* parent );

        /** Release a view. */
        virtual void releaseView( View* view );

        /** 
         * Create a new canvas.
         * 
         * @return the canvas.
         */
        virtual Canvas* createCanvas( Config* parent );

        /** Release a canvas. */
        virtual void releaseCanvas( Canvas* canvas );

        /** 
         * Create a new segment.
         * 
         * @return the segment.
         */
        virtual Segment* createSegment( Canvas* parent );

        /** Release a segment. */
        virtual void releaseSegment( Segment* segment );

        /** 
         * Create a new pipe.
         * 
         * @return the pipe.
         */
        virtual Pipe* createPipe( Node* parent );

        /** Release a pipe. */
        virtual void releasePipe( Pipe* pipe );

        /** 
         * Create a new window.
         * 
         * @return the window.
         */
        virtual Window* createWindow( Pipe* parent );

        /** Release a window. */
        virtual void releaseWindow( Window* window );

        /** 
         * Create a new channel.
         * 
         * @return the channel.
         */
        virtual Channel* createChannel( Window* parent );

        /** Release a channel. */
        virtual void releaseChannel( Channel* channel );

        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H


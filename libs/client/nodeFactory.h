
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

#include <eq/types.h>
#include <eq/api.h>

namespace eq
{
    /**
     * The node factory is a per-node singleton used to create and release
     * Equalizer resource instances.
     *
     * The instances have to be subclasses of the corresponding Equalizer
     * classes, and are used to selectively override task methods and store
     * additional, application-specific data.
     *
     * @sa eq::init()
     */
    class EQ_API NodeFactory
    {
    public:
        /** 
         * Create a new config.
         * 
         * @return the config.
         * @version 1.0
         */
        virtual Config* createConfig( ServerPtr parent );

        /** Release a config. @version 1.0 */
        virtual void releaseConfig( Config* config );

        /** 
         * Create a new node.
         * 
         * @return the node.
         * @version 1.0
         */
        virtual Node* createNode( Config* parent );

        /** Release a node. @version 1.0 */
        virtual void releaseNode( Node* node );

        /** 
         * Create a new observer.
         * 
         * @return the observer.
         * @version 1.0
         */
        virtual Observer* createObserver( Config* parent );

        /** Release a observer. @version 1.0 */
        virtual void releaseObserver( Observer* observer );

        /** 
         * Create a new layout.
         * 
         * @return the layout.
         * @version 1.0
         */
        virtual Layout* createLayout( Config* parent );

        /** Release a layout. @version 1.0 */
        virtual void releaseLayout( Layout* layout );

        /** 
         * Create a new view.
         * 
         * @return the view.
         * @version 1.0
         */
        virtual View* createView( Layout* parent );

        /** Release a view. @version 1.0 */
        virtual void releaseView( View* view );

        /** 
         * Create a new canvas.
         * 
         * @return the canvas.
         * @version 1.0
         */
        virtual Canvas* createCanvas( Config* parent );

        /** Release a canvas. @version 1.0 */
        virtual void releaseCanvas( Canvas* canvas );

        /** 
         * Create a new segment.
         * 
         * @return the segment.
         * @version 1.0
         */
        virtual Segment* createSegment( Canvas* parent );

        /** Release a segment. @version 1.0 */
        virtual void releaseSegment( Segment* segment );

        /** 
         * Create a new pipe.
         * 
         * @return the pipe.
         * @version 1.0
         */
        virtual Pipe* createPipe( Node* parent );

        /** Release a pipe. @version 1.0 */
        virtual void releasePipe( Pipe* pipe );

        /** 
         * Create a new window.
         * 
         * @return the window.
         * @version 1.0
         */
        virtual Window* createWindow( Pipe* parent );

        /** Release a window. @version 1.0 */
        virtual void releaseWindow( Window* window );

        /** 
         * Create a new channel.
         * 
         * @return the channel.
         * @version 1.0
         */
        virtual Channel* createChannel( Window* parent );

        /** Release a channel. @version 1.0 */
        virtual void releaseChannel( Channel* channel );

        /** Destruct this node factory. @version 1.0 */
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H


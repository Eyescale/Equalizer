
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_CHANNEL_H
#define EQFABRIC_CHANNEL_H

#include <eq/fabric/object.h>        // base class

#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
namespace fabric
{
    class ChannelVisitor;
    class Window;
    template< typename T > class LeafVisitor;

    /**
     * A channel represents a two-dimensional viewport within a Window.
     *
     * The channel is the basic rendering entity. It represents a 2D rendering
     * area within a Window. It executes all rendering-relevant tasks, such as
     * clear, draw, assemble and readback. Each rendering task is using its own
     * RenderContext, which is computed by the server based on the rendering
     * description of the current configuration.
     */
    template< typename T, typename W > class Channel : public Object
    {
    public:
        /** Construct a new channel. @version 1.0 */
        EQ_EXPORT Channel( W* parent );

        /** Destruct the channel. @version 1.0 */
        EQ_EXPORT virtual ~Channel();

        /**
         * @name Data Access
         */
        //@{
        /** @return the parent window. @version 1.0 */
        W* getWindow() { return _window; }

        /** @return the parent window. @version 1.0 */
        const W* getWindow() const { return _window; }

        /** 
         * Traverse this channel using a channel visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQ_EXPORT VisitorResult accept( LeafVisitor< T > & visitor );

        /** Const-version of accept(). @version 1.0 */
        EQ_EXPORT VisitorResult accept( LeafVisitor< T >& visitor ) const;

    protected:
        /** The parent window. */
        W* const _window;
        friend class Window;

    private:
        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }
    };
}
}

#endif // EQFABRIC_CHANNEL_H


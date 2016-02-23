
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQS_TREEEQUALIZER_H
#define EQS_TREEEQUALIZER_H

#include "../channelListener.h" // base class
#include "equalizer.h"          // base class

#include <eq/fabric/range.h>    // member
#include <eq/fabric/viewport.h> // member

#include <deque>
#include <vector>

namespace eq
{
namespace server
{
    std::ostream& operator << ( std::ostream& os, const TreeEqualizer* );

    /** Adapts the 2D tiling or DB range of the attached compound's children. */
    class TreeEqualizer : public Equalizer, protected ChannelListener
    {
    public:
        EQSERVER_API TreeEqualizer();
        TreeEqualizer( const TreeEqualizer& from );
        virtual ~TreeEqualizer();
        void toStream( std::ostream& os ) const final { os << this; }

        /** @sa CompoundListener::notifyUpdatePre */
        void notifyUpdatePre( Compound* compound,
                              const uint32_t frameNumber ) final;

        /** @sa ChannelListener::notifyLoadData */
        void notifyLoadData( Channel* channel, uint32_t frameNumber,
                             const Statistics& statistics,
                             const Viewport& region ) final;

        uint32_t getType() const final { return fabric::TREE_EQUALIZER; }

    protected:
        void notifyChildAdded( Compound*, Compound* ) override
            { LBASSERT( !_tree ); }
        void notifyChildRemove( Compound*, Compound* ) override
            { LBASSERT( !_tree ); }

    private:
        struct Node
        {
            Node() : left(0), right(0), compound(0), mode( MODE_VERTICAL )
                   , resources( 0.0f ), split( 0.5f ), oldsplit( 0.0f ), boundaryf( 0.0f )
                   , resistancef( 0.0f ), time( 1 ) {}
            ~Node() { delete left; delete right; }

            Node*     left;      //<! Left child (only on non-leafs)
            Node*     right;     //<! Right child (only on non-leafs)
            Compound* compound;  //<! The corresponding child (only on leafs)
            TreeEqualizer::Mode mode; //<! What to adapt
            float     resources; //<! total amount of resources of subtree
            float     split;     //<! 0..1 local (vp, range) split
            float     oldsplit;
            float     boundaryf;
            Vector2i  boundary2i;
            float     resistancef;
            Vector2i  resistance2i;
            Vector2i  maxSize;
            int64_t   time;
        };
        friend std::ostream& operator << ( std::ostream& os, const Node* node );
        typedef std::vector< Node* > LBNodes;

        Node* _tree; // <! The binary split tree of all children

        //-------------------- Methods --------------------
        /** @return true if we have a valid LB tree */
        Node* _buildTree( const Compounds& children );

        /** Clear the tree, does not delete the nodes. */
        void _clearTree( Node* node );

        void _notifyLoadData( Node* node, Channel* channel,
                              const Statistics& statistics );

        /** Update all node fields influencing the split */
        void _update( Node* node );

        /** Adjust the split of each node based on the front-most _history. */
        void _split( Node* node );
        void _assign( Node* node, const Viewport& vp, const Range& range );
    };
}
}

#endif // EQS_TREEEQUALIZER_H

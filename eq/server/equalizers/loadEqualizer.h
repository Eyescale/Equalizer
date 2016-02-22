
/* Copyright (c) 2008-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQS_LOADEQUALIZER_H
#define EQS_LOADEQUALIZER_H

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
std::ostream& operator << ( std::ostream& os, const LoadEqualizer* );

/** Adapts the 2D tiling or DB range of the attached compound's children. */
class LoadEqualizer : public Equalizer, protected ChannelListener
{
public:
    EQSERVER_API LoadEqualizer();
    explicit LoadEqualizer( const fabric::Equalizer& from );
    virtual ~LoadEqualizer();
    void toStream( std::ostream& os ) const final { os << this; }

    /** @sa CompoundListener::notifyUpdatePre */
    void notifyUpdatePre( Compound* compound,
                          const uint32_t frameNumber ) final;

    /** @sa ChannelListener::notifyLoadData */
    void notifyLoadData( Channel* channel,
                         uint32_t frameNumber,
                         const Statistics& statistics,
                         const Viewport& region ) final;

    uint32_t getType() const final { return fabric::LOAD_EQUALIZER; }

protected:
    void notifyChildAdded( Compound*, Compound* ) override
    { LBASSERT( !_tree ); }
    void notifyChildRemove( Compound*, Compound* ) override
    { LBASSERT( !_tree ); }

private:
    struct Node
    {
        Node() : left(0), right(0), compound(0), mode( MODE_VERTICAL )
               , resources( 0.0f ), split( 0.5f ), boundaryf( 0.0f )
               , resistancef( 0.0f ) {}
        ~Node() { delete left; delete right; }

        Node*     left;      //<! Left child (only on non-leafs)
        Node*     right;     //<! Right child (only on non-leafs)
        Compound* compound;  //<! The corresponding child (only on leafs)
        LoadEqualizer::Mode mode; //<! What to adapt
        float     resources; //<! total amount of resources of subtree
        float     split;     //<! 0..1 global (vp, range) split
        float     boundaryf;
        Vector2i  boundary2i;
        float     resistancef;
        Vector2i  resistance2i;
        Vector2i  maxSize;
    };
    friend std::ostream& operator << ( std::ostream& os, const Node* node );
    typedef std::vector< Node* > LBNodes;

    Node* _tree; // <! The binary split tree of all children

    struct Data
    {
        Data() : channel( 0 ), taskID( 0 ), destTaskID( 0 )
               , time( -1 ), assembleTime( 0 ) {}
        Channel* channel;
        uint32_t taskID;
        uint32_t destTaskID;
        Viewport vp;
        Range    range;
        int64_t  time;
        int64_t  assembleTime;
    };

    typedef std::vector< Data > LBDatas;
    typedef std::pair< uint32_t,  LBDatas > LBFrameData;

    std::deque< LBFrameData > _history;

    //-------------------- Methods --------------------
    /** @return true if we have a valid LB tree */
    Node* _buildTree( const Compounds& children );

    /** Setup assembly with the compound dest value */
    void _updateAssembleTime( Data& data, const Statistic& stat );

    /** Clear the tree, does not delete the nodes. */
    void _clearTree( Node* node );

    /** get the total time used by the rendering. */
    int64_t _getTotalTime();

    /** get the assembly time used by the compound which use
        the destination Channel. */
    int64_t _getAssembleTime( );

    /** Obsolete _history so that front-most item is youngest available. */
    void _checkHistory();

    /** Update all node fields influencing the split */
    void _update( Node* node, const Viewport& vp, const Range& range );
    void _updateLeaf( Node* node );
    void _updateNode( Node* node, const Viewport& vp, const Range& range );

    /** Adjust the split of each node based on the front-most _history. */
    void _computeSplit();
    void _removeEmpty( LBDatas& items );

    void _computeSplit( Node* node, const float time, LBDatas* sortedData,
                        const Viewport& vp, const Range& range );
    void _assign( Compound* compound, const Viewport& vp,
                  const Range& range );

    /** Get the resource for all children compound. */
    float _getTotalResources( ) const;

    static bool _compareX( const Data& data1, const Data& data2 )
    { return data1.vp.x < data2.vp.x; }
    static bool _compareY( const Data& data1, const Data& data2 )
    { return data1.vp.y < data2.vp.y; }
    static bool _compareRange( const Data& data1, const Data& data2 )
    { return data1.range.start < data2.range.start; }
};
}
}

#endif // EQS_LOADEQUALIZER_H

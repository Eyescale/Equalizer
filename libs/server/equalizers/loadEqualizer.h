
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/types.h>
#include <eq/fabric/range.h>    // member
#include <eq/fabric/viewport.h> // member

#include <deque>
#include <vector>

namespace eq
{
namespace server
{
    class LoadEqualizer;
    std::ostream& operator << ( std::ostream& os, const LoadEqualizer* );

    /** Adapts the 2D tiling or DB range of the attached compound's children. */
    class LoadEqualizer : public Equalizer, protected ChannelListener
    {
    public:
        EQSERVER_EXPORT LoadEqualizer();
        LoadEqualizer( const LoadEqualizer& from );
        virtual ~LoadEqualizer();
        virtual Equalizer* clone() const { return new LoadEqualizer( *this ); }
        virtual void toStream( std::ostream& os ) const { os << this; }

        enum Mode
        {
            MODE_DB = 0,     //!< Adapt for a sort-last decomposition
            MODE_HORIZONTAL, //!< Adapt for sort-first using horizontal stripes
            MODE_VERTICAL,   //!< Adapt for sort-first using vertical stripes
            MODE_2D          //!< Adapt for a sort-first decomposition
        };

        /** Set the load balancer adaptation mode. */
        void setMode( const Mode mode ) { _mode = mode; }

        /** @return the load balancer adaptation mode. */
        Mode getMode() const { return _mode; }

        /** Set the damping factor for the viewport or range adjustment.  */
        void setDamping( const float damping ) { _damping = damping; }

        /** @return the damping factor. */
        float getDamping() const { return _damping; }

        /** @sa CompoundListener::notifyUpdatePre */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber );

        /** @sa ChannelListener::notifyLoadData */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber, 
                                     const uint32_t nStatistics,
                                     const eq::Statistic* statistics );
                                     
        /** Set a boundary for 2D tiles. */
        void setBoundary( const Vector2i& boundary )
        {
            EQASSERT( boundary.x() > 0 && boundary.y() > 0 );
            _boundary2i = boundary; 
        }

        /** Set a boundary for DB ranges. */
        void setBoundary( const float boundary )
        { 
            EQASSERT( boundary > 0.0f );
            _boundaryf = boundary; 
        }

        /** @return the boundary for 2D tiles. */
        const Vector2i& getBoundary2i() const { return _boundary2i; }

        /** @return the boundary for DB ranges. */
        float getBoundaryf() const { return _boundaryf; }

        void setAssembleOnlyLimit( const float limit )
            { _assembleOnlyLimit = limit; }

    protected:
        virtual void notifyChildAdded( Compound* compound, Compound* child )
            { EQASSERT( !_tree ); }
        virtual void notifyChildRemove( Compound* compound, Compound* child )
            { EQASSERT( !_tree ); }

    private:
        Mode  _mode;    //!< The current adaptation mode
        float _damping; //!< The damping factor,  (0: No damping, 1: No changes)
        
        struct Node
        {
            Node() : left(0), right(0), compound(0), mode( MODE_VERTICAL )
                   , resources( 0.0f ), split( 0.5f ), boundaryf( 0.0f ) {}
            ~Node() { delete left; delete right; }

            Node*     left;      //<! Left child (only on non-leafs)
            Node*     right;     //<! Right child (only on non-leafs)
            Compound* compound;  //<! The corresponding child (only on leafs)
            LoadEqualizer::Mode mode; //<! What to adapt
            float     resources; //<! total amount of resources of subtree
            float     split;     //<! 0..1 global (vp, range) split
            float     boundaryf;
            Vector2i  boundary2i;
            Vector2i  maxSize;
        };
        friend std::ostream& operator << ( std::ostream& os, const Node* node );
        typedef std::vector< Node* > LBNodes;

        Node* _tree; // <! The binary split tree of all children

        struct Data
        {
            Data() : channel( 0 ), taskID( 0 ), destTaskID( 0 )
                   , time( -1 ), assembleTime( 0 ) {}
            Channel*     channel;
            uint32_t     taskID;
            uint32_t     destTaskID;
            eq::Viewport vp;
            eq::Range    range;
            int64_t      time;
            int64_t      assembleTime;
            float        load;          //<! time/vp.area
        };

        typedef std::vector< Data > LBDatas;
        typedef std::pair< uint32_t,  LBDatas > LBFrameData;
        
        std::deque< LBFrameData > _history;

        Vector2i _boundary2i;  // default: 1 1
        float    _boundaryf;   // default: numeric_limits<float>::epsilon
        float    _assembleOnlyLimit; // default: numeric_limits<float>::max

        //-------------------- Methods --------------------
        /** @return true if we have a valid LB tree */
        Node* _buildTree( const Compounds& children );
        void _init( Node* node, const Viewport& vp, const Range& range );

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
        void _update( Node* node );
        void   _updateLeaf( Node* node );
        void   _updateNode( Node* node );

        /** Adjust the split of each node based on the front-most _history. */
        void _computeSplit();
        void _removeEmpty( LBDatas& items );

        void _computeSplit( Node* node, const float time, LBDatas* sortedData,
                            const eq::Viewport& vp, const eq::Range& range );
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

    std::ostream& operator << ( std::ostream& os, const LoadEqualizer::Mode );
}
}

#endif // EQS_LOADEQUALIZER_H

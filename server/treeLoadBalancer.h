
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_TREELOADBALANCER_H
#define EQS_TREELOADBALANCER_H

#include "channelListener.h" // base class
#include "loadBalancer.h"    // base class LoadBalancerIF

#include <eq/client/range.h>
#include <eq/client/viewport.h>

#include <deque>
#include <vector>

namespace eq
{
namespace server
{
    /** Adapts the 2D tiling or range of the associated compound's children. */
    class TreeLoadBalancer : public LoadBalancerIF, protected ChannelListener
    {
    public:
        TreeLoadBalancer( const LoadBalancer& parent );
        virtual ~TreeLoadBalancer();

        /** @sa LoadBalancerIF::update */
        virtual void update( const uint32_t frameNumber );

        /** @sa ChannelListener::notifyLoadData */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const float startTime, const float endTime
                                     /*, const float load */ );

    private:
        struct Node
        {
            Node() : left(0), right(0), compound(0), 
                     splitMode( LoadBalancer::MODE_VERTICAL ) , time( 0.0f ) {}
            ~Node() { delete left; delete right; }

            Node*     left;      //<! Left child (only on non-leafs)
            Node*     right;     //<! Right child (only on non-leafs)
            Compound* compound;  //<! The corresponding child (only on leafs)
            LoadBalancer::Mode splitMode; //<! What to adapt
            float     time;      //<! target render time for next frame
        };
        friend std::ostream& operator << ( std::ostream& os,
                                           const TreeLoadBalancer::Node* node );
        typedef std::vector< Node* > LBNodeVector;

        Node* _tree; // <! The binary split tree of all children

        struct Data
        {
            Data() : compound(0), startTime( -1.f ), endTime( -1.f ) {}

            Compound*    compound;
            eq::Viewport vp;
            eq::Range    range;
            float        startTime;
            float        endTime;
            float        load;          //<! (endTime-startTime)/vp.area
        };

        typedef std::vector< Data >                  LBDataVector;
        typedef std::deque< Data >                   LBDataDeque;
        typedef std::pair< uint32_t,  LBDataVector > LBFrameData;
        
        std::deque< LBFrameData > _history;
        
        bool _freeze;
        
        //-------------------- Methods --------------------
        /** @return true if we have a valid LB tree */
        Node* _buildTree( const CompoundVector& children );

        /** Clear the tree, does not delete the nodes. */
        void _clearTree( Node* node );

        /** Obsolete _history so that front-most item is youngest available. */
        void _checkHistory();
        
        /** Adjust the split of each node based on the front-most _history. */
        void _computeSplit();
        float _assignTargetTimes( Node* node, const float totalTime, 
                                  const float resourceTime );
        void _computeSplit( Node* node, LBDataVector* sortedData,
                            const eq::Viewport& vp, const eq::Range& range );

        static bool _compareX( const TreeLoadBalancer::Data& data1,
                               const TreeLoadBalancer::Data& data2 )
            { return data1.vp.x < data2.vp.x; }

        static bool _compareY( const TreeLoadBalancer::Data& data1,
                               const TreeLoadBalancer::Data& data2 )
            { return data1.vp.y < data2.vp.y; }

        static bool _compareRange( const TreeLoadBalancer::Data& data1,
                                   const TreeLoadBalancer::Data& data2 )
            { return data1.range.start < data2.range.start; }
    };
}
}

#endif // EQS_LOADBALANCER_H

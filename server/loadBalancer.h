
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADBALANCER_H
#define EQS_LOADBALANCER_H

#include "channelListener.h" // base class
#include "compoundListener.h" // base class
#include "types.h"

#include <eq/client/range.h>
#include <eq/client/viewport.h>

#include <deque>
#include <vector>

namespace eq
{
namespace server
{
    /** Adapts the 2D tiling of the associated compound's children. */
    class EQSERVER_EXPORT LoadBalancer : protected CompoundListener,
                                         protected ChannelListener
    {
    public:
        LoadBalancer();
        LoadBalancer( const LoadBalancer& from );
        virtual ~LoadBalancer();

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

        /** Attach to a compound and detach the previous compound. */
        void attach( Compound* compound );

        /** @sa CompoundListener::notifyUpdatePre */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber );

        /** @sa CompoundListener::notifyChildAdded */
        virtual void notifyChildAdded( Compound* compound, Compound* child );

        /** @sa CompoundListener::notifyChildRemove */
        virtual void notifyChildRemove( Compound* compound, Compound* child );

        /** @sa ChannelListener::notifyLoadData */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const float startTime, const float endTime
                                     /*, const float load */ );

        void setFreeze( const bool onOff ) { _freeze = onOff; }

    private:
        //-------------------- Members --------------------
        Mode _mode; //!< The current adaptation mode

        Compound* _compound; //!< The attached compound

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
                                           const LoadBalancer::Node* node );
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
        /** Clear all cached data and old load data sets. */
        void _clear();

        /** @return true if we have a valid LB tree */
        Node* _buildTree( const CompoundVector& children );

        /** Clear the tree, does not delete the nodes. */
        void _clearTree( Node* node );

        /** Obsolete _history so that front-most item is youngest available. */
        void _checkHistory();
        
        /** Adjust the split of each node based on the front-most _history. */
        void _computeSplit();
        float _assignIdleTimes( Node* node, const LBDataVector& items,
                                const float maxIdleTime, const float endTime );
        float _assignTargetTimes( Node* node, const float totalTime, 
                                  const float resourceTime );
        void _computeSplit( Node* node, LBDataVector* sortedData,
                            const eq::Viewport& vp, const eq::Range& range );

        static bool _compareX( const LoadBalancer::Data& data1,
                               const LoadBalancer::Data& data2 )
            { return data1.vp.x < data2.vp.x; }

        static bool _compareY( const LoadBalancer::Data& data1,
                               const LoadBalancer::Data& data2 )
            { return data1.vp.y < data2.vp.y; }

        static bool _compareRange( const LoadBalancer::Data& data1,
                                   const LoadBalancer::Data& data2 )
            { return data1.range.start < data2.range.start; }
    };

    std::ostream& operator << ( std::ostream& os, const LoadBalancer* lb );
}
}

#endif // EQS_LOADBALANCER_H

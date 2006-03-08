
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include "channel.h"

#include <eq/client/frustum.h>
#include <eq/client/projection.h>
#include <eq/client/viewport.h>
#include <eq/client/wall.h>

#include <iostream>
#include <vector>

namespace eqs
{
    enum TraverseResult
    {
        TRAVERSE_CONTINUE,
        TRAVERSE_TERMINATE
    };

    /**
     * The compound.
     */
    class Compound
    {
    public:
        /** 
         * Constructs a new Compound.
         */
        Compound();

        /**
         * Constructs a new, deep copy of the passed compound
         */
        Compound( const Compound& from );

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Adds a new child to this compound.
         * 
         * @param child the child.
         */
        void addChild( Compound* child );

        /** 
         * Removes a child from this compound.
         * 
         * @param child the child
         * @return <code>true</code> if the child was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeChild( Compound* child );

        /** 
         * Returns the number of children on this compound.
         * 
         * @return the number of children on this compound. 
         */
        uint32_t nChildren() const { return _children.size(); }

        /** 
         * Return if the compound is a leaf compound.
         * @return if the compound is a leaf compound. 
         */
        bool isLeaf() const { return _children.empty(); }

        /** 
         * Gets a child.
         * 
         * @param index the child's index. 
         * @return the child.
         */
        Compound* getChild( const uint32_t index ) const
            { return _children[index]; }

        /** 
         * Gets the parent compound.
         * 
         * @return the parent compound.
         */
        Compound* getParent() const
            { return _parent; }

        /** 
         * Set the channel of this compound.
         *
         * The compound uses the channel for all rendering operations executed
         * by this compound.
         * 
         * @param channel the channel.
         */
        void setChannel( Channel* channel ){ _data.channel = channel; }

        /** 
         * Returns the channel of this compound.
         * 
         * @return the channel of this compound.
         */
        Channel* getChannel() const { return _data.channel; }

        Window* getWindow() const 
            { return _data.channel ? _data.channel->getWindow() : NULL; }

        /**
         * The decomposition mode of the compound.
         */
        enum Mode
        {
            MODE_SYNC   //!< Synchronize swap of all channels
        };

        /** 
         * Set the decomposition mode.
         * 
         * @param mode the decomposition mode.
         */
        void setMode( const Mode mode ) { _mode = mode; }

        /** 
         * Return the decomposition mode.
         *
         * @return the decomposition mode.
         */
        Mode getMode() const { return _mode; }
        //*}

        /**
         * @name View Operations
         */
        //*{
        /** 
         * Set the compound's view frustum using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const eq::Wall& wall );

        /** 
         * Set the compound's view frustum using a projection description
         * 
         * @param projection the projection description.
         */
        void setProjection( const eq::Projection& projection );

        /** 
         * Set the compound's view frustum as a four-by-four matrix.
         * 
         * @param frustum the frustum description.
         */
        void setFrustum( const eq::Frustum& frustum );
        //*}

        /**
         * @name Compound Operations
         */
        //*{
        typedef TraverseResult (*TraverseCB)(Compound* compound,void* userData);

        /** 
         * Traverses a compound tree in a top-down, left-to-right order.
         * 
         * @param compound the top level compound of the tree to traverse.
         * @param preCB the callback to execute for non-leaf compounds when
         *              traversing down, can be <code>NULL</code>.
         * @param leafCB the callback to execute for leaf compounds, can be
         *               <code>NULL</code>
         * @param postCB the callback to execute for non-leaf compounds when
         *              traversing up, can be <code>NULL</code>.
         * @param userData an opaque pointer passed through to the callbacks.
         *
         * @return the result of the traversal, <code>TRAVERSE_TERMINATE</code>
         *         if the traversal was terminated by one of the callbacks, 
         *         otherwise <code>TRAVERSE_CONTINUE</code> .
         */
        static TraverseResult traverse( Compound* compound, TraverseCB preCB,
                                        TraverseCB leafCB, TraverseCB postCB,
                                        void* userData );

        /** 
         * Initialises this compound.
         */
        void init();

        /** 
         * Exits this compound.
         */
        void exit();

        /** 
         * Updates this compound.
         * 
         * The compound's parameters for the next frame are computed.
         */
        void update();

        /** 
         * Update a channel by generating all rendering tasks for this frame.
         * 
         * @param channel the channel to update.
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void updateChannel( Channel* channel, const uint32_t frameID );
        //*}

    private:
        Compound               *_parent;
        std::vector<Compound*>  _children;
        
        Compound* _getNext() const;

        struct View
        {
            enum Type
            {
                NONE,
                WALL,
                PROJECTION,
                FRUSTUM
            };

            View() : latest( NONE ) {}

            Type           latest;
            eq::Wall       wall;
            eq::Projection projection;
            eq::Frustum    frustum;
        } 
        _view;

        struct InheritData
        {
            InheritData();

            Channel*     channel;
            eq::Viewport vp;
            eq::Frustum  frustum;
        };

        InheritData _data;
        InheritData _inherit;

        Mode        _mode;


        void _updateInheritData();

        static TraverseResult _updateDrawCB(Compound* compound, void* );
        static TraverseResult _updatePostDrawCB( Compound* compound, void* );

        void _updateSwapGroup();
        void _computeFrustum( float frustum[6], float headTransform[16] );
    };

    std::ostream& operator << (std::ostream& os,const Compound* compound);
};
#endif // EQS_COMPOUND_H


/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include <iostream>
#include <vector>

// Definitions for common display systems in meters
// to be moved to client lib
#define WALL_20INCH_16x10 {                            \
        { -.21672, -.13545, -1, },                     \
        {  .21672, -.13545, -1, },                     \
        { -.21672,  .13545, -1, }}

namespace eqs
{
    class Channel;

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
        void setChannel( Channel* channel ){ _channel = channel; }

        /** 
         * Returns the channel of this compound.
         * 
         * @return the channel of this compound.
         */
        Channel* getChannel() const { return _channel; }

        /**
         * @name View Operations
         */
        //*{
        /**
         * A wall definition defining a view frustum.
         * 
         * The three points describe the bottom left, bottom right and top left
         * coordinate of the wall in real-world coordinates.
         */
        struct Wall // to be moved to client library 
        {
            float bottomLeft[3];
            float bottomRight[3];
            float topLeft[3];
        };

        /**
         * A projection definition defining a view frustum.
         * 
         * The frustum is defined by a projection system positioned at origin,
         * orientated as defined by the head-pitch-roll angles projecting to a
         * wall at the given distance. The fov defines the horizontal and
         * vertical field of view of the projector.
         */
        struct Projection // to be moved to client library 
        {
            float origin[3];
            float distance;
            float fov[2];
            float hpr[3];
        };

        /** 
         * A generic frustum definition
         *
         * The frustum is defined by the size of the viewport and a
         * transformation matrix.
         * @todo better doc
         */
        struct Frustum
        {
            float width;
            float height;
            float xfm[16];
        };

        /** 
         * Set the compound's view frustum using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const Wall& wall );

        /** 
         * Set the compound's view frustum using a projection description
         * 
         * @param projection the projection description.
         */
        void setProjection( const Projection& projection );

        /** 
         * Set the compound's view frustum as a four-by-four matrix.
         * 
         * @param frustum the frustum description.
         */
        void setFrustum( const Frustum& frustum );
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
         * All tasks are send to produce a new frame of rendering for this
         * compound tree.
         */
        void update();
        //*}

    private:
        Compound               *_parent;
        std::vector<Compound*>  _children;
        
        Channel* _channel;
        
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

            Type        latest;
            Wall        wall;
            Projection  projection;
            Frustum     frustum;
        } 
        _view;

        Frustum     _frustum;

        struct
        {
            Channel* channel;
            int      pvp[4];
            int      vp[4];
        }
        _inherit;
    };

    std::ostream& operator << (std::ostream& os,const Compound* compound);
};
#endif // EQS_COMPOUND_H

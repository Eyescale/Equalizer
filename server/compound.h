
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include <iostream>
#include <vector>

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
        uint nChildren() const { return _children.size(); }

        /** 
         * Gets a child.
         * 
         * @param index the child's index. 
         * @return the child.
         */
        Compound* getChild( const uint index ) const
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

        typedef TraverseResult (*TraverseCB)(Compound* compound,void* userData);

        static TraverseResult traverse( Compound* compound, TraverseCB preCB,
                                        TraverseCB leafCB, TraverseCB postCB,
                                        void* userData );

        /**
         * @name Compound Operations
         */
        //*{

        /** 
         * Initialises this compound.
         */
        void init();

        //*}

    private:
        Compound               *_parent;
        std::vector<Compound*>  _children;
        
        Channel* _channel;
        
        Compound* _getNext() const;
    };

    std::ostream& operator << (std::ostream& os,const Compound* compound);
};
#endif // EQS_COMPOUND_H

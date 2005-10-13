
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include <iostream>
#include <vector>

namespace eqs
{
    class Channel;

    /**
     * The compound.
     */
    class Compound
    {
    public:
        /** 
         * Constructs a new Compound.
         */
        Compound() : _channel( NULL ) {}

        /** 
         * Constructs a new deep copy of another compound.
         * 
         * @param from the original compound.
         */
        Compound(const Compound& from);

        /** 
         * Adds a new compound to this compound.
         * 
         * @param compound the compound.
         */
        void addCompound( Compound* compound ){ _compounds.push_back(compound);}

        /** 
         * Removes a compound from this compound.
         * 
         * @param compound the compound
         * @return <code>true</code> if the compound was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeCompound( Compound* compound );

        /** 
         * Returns the number of compounds on this compound.
         * 
         * @return the number of compounds on this compound. 
         */
        uint nCompounds() const { return _compounds.size(); }

        /** 
         * Gets a compound.
         * 
         * @param index the compound's index. 
         * @return the compound.
         */
        Compound* getCompound( const uint index ) const
            { return _compounds[index]; }

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
         * @name Compound Operations
         */
        //*{

        /** 
         * Initialises this compound.
         */
        void init();

        //*}

    private:
        std::vector<Compound*> _compounds;

        Channel* _channel;
    };

    inline std::ostream& operator << (std::ostream& os,const Compound* compound)
    {
        if( !compound )
        {
            os << "NULL compound";
            return os;
        }

        const uint nCompounds = compound->nCompounds();
        os << "compound " << (void*)compound << " channel " 
           << compound->getChannel() << " " << nCompounds << " children";

        for( uint i=0; i<nCompounds; i++ )
            os << std::endl << "    " << compound->getCompound(i);

        return os;
    }
};
#endif // EQS_COMPOUND_H

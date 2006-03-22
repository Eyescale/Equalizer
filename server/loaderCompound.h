
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_COMPOUND_H
#define EQS_LOADER_COMPOUND_H

namespace eqLoader
{
    struct newCompoundAction
    {
        newCompoundAction( State& state ) : _state( state ) {}

        void operator()(const char& c) const
            {
                Compound* compound = _state.loader->createCompound( );
                if( !_state.compound )
                    _state.config->addCompound( compound );
                else
                    _state.compound->addChild( compound );

                _state.compound = compound;
            }
    
        State& _state;
    };

    struct leaveCompoundAction
    {
        leaveCompoundAction( State& state ) : _state( state ) {}

        void operator()(const char& c) const
            {
                _state.compound = _state.compound->getParent();
            }
    
        State& _state;
    };

    struct compoundSetModeAction
    {
        compoundSetModeAction( State& state, const Compound::Mode mode ) :
                _state( state ), _mode( mode ) {}

        template <typename IteratorT>
        void operator()(IteratorT first, IteratorT last) const
            {
                _state.compound->setMode( _mode );
            }
    
        State&         _state;
        Compound::Mode _mode;
    };
}

#endif // EQS_LOADER_COMPOUND_H


/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_PIPE_AGL_H
#define EQ_OS_PIPE_AGL_H

#include <eq/client/osPipe.h> // base class

namespace eq
{
    /** Equalizer default implementation of an AGL pipe */
    class EQ_EXPORT AGLPipe : public OSPipe
    {
    public:
        AGLPipe( Pipe* parent );
        virtual ~AGLPipe( );

        //* @name AGL initialization
        //*{
        /** 
         * Initialize this pipe for the AGL window system.
         * 
         * @return true if the initialization was successful, false otherwise.
         */
        virtual bool configInit( );

        /** 
         * Deinitialize this pipe for the AGL window system.
         * 
         * @return true if the deinitialization was successful, false otherwise.
         */
        virtual void configExit( );
        //*}

        /** @return the CG display ID for this pipe. */
        CGDirectDisplayID getCGDisplayID() const { return _cgDisplayID; }

    private:

        /** @name Data Access. */
        //*{
        /** 
         * Set the CG display ID for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param id the CG display ID for this pipe.
         */
        void _setCGDisplayID( CGDirectDisplayID id );
        //*}

        /** Window-system specific display information. */
        CGDirectDisplayID _cgDisplayID;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}

#endif // EQ_OS_PIPE_AGL_H

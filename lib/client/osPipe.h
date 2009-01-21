
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
   All rights reserved. */

#ifndef EQ_OS_PIPE_H
#define EQ_OS_PIPE_H

#include "pipe.h"


namespace eq
{
    /**
     * The interface definition for OS-specific pipe code.
     *
     * The OSPipe abstracts all pipe system specific code and facilitates
     * porting to new windowing systems. Each Pipe uses one OSPipe, which
     * is initialized in Pipe::configInitOSPipe.
     */

    class EQ_EXPORT OSPipe
    {
    public:

        OSPipe( Pipe* parent )
            : _pipe( parent )
        {
            EQASSERT( _pipe );
        }

        virtual ~OSPipe( ) {}

        /** @name Methods forwarded from eq::Pipe */
        //*{
        virtual bool configInit( ) = 0;

        virtual void configExit( ) = 0;
        //*}

        /** @return the reason of the last error. */
        const std::string & getErrorMessage() const { return _error; }

        Pipe* getPipe() { return _pipe; }
        const Pipe* getPipe() const { return _pipe; }

    protected:
        /** @name Error information. */
        //*{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //*}

        /** The parent eq::Pipe. */
        Pipe* const _pipe;

        /** The reason for the last error. */
        std::string _error;
    };
}

#endif //EQ_OS_PIPE_H


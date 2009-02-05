
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPESTATISTICS_H
#define EQ_PIPESTATISTICS_H

#include <eq/client/configEvent.h> // member

namespace eq
{
    class Pipe;

    /**
     * Holds one statistics event, used for profiling.
     */
    class PipeStatistics
    {
    public:
        PipeStatistics( const Statistic::Type type, Pipe* pipe );
        ~PipeStatistics();

        ConfigEvent event;

    private:
        Pipe* const _pipe;
    };
}

#endif // EQ_PIPESTATISTICS_H

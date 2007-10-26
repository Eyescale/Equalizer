
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_NODE_H
#define EVOLVE_NODE_H

#include "eVolve.h"
#include "initData.h"

#include <eq/eq.h>

//#include "rawVolModel.h"

namespace eVolve
{

    class Node : public eq::Node
    {
    public:
        const InitData& getInitData() const { return _initData; }

    protected:
        virtual ~Node(){}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
        virtual void frameDrawFinish( const uint32_t frameID,
                                      const uint32_t frameNumber )
            { /* nop, see frameStart */ }

    private:
        InitData _initData;
    };

}

#endif // EVOLVE_NODE_H

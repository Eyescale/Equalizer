
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "loadBalancer.h"

#include "compound.h"
#include "log.h"
#include "treeLoadBalancer.h"
#include "smoothLoadBalancer.h"
#include "dfrLoadBalancer.h"

#include <eq/client/client.h>
#include <eq/client/server.h>
#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

LoadBalancer::LoadBalancer()
        : _mode( MODE_2D )
        , _damping( -1.f )
        , _frameRate( 10.f )
        , _compound( 0 )
        , _implementation( 0 )
        , _freeze( false )
{
    EQINFO << "New LoadBalancer @" << (void*)this << endl;
}

LoadBalancer::LoadBalancer( const LoadBalancer& from )
        : CompoundListener()
        , _mode( from._mode )
        , _damping( from._damping )
        , _frameRate( from._frameRate )
        , _compound( 0 )
        , _implementation( 0 )
        , _freeze( from._freeze )
{
}

LoadBalancer::~LoadBalancer()
{
    attach( 0 );
}

void LoadBalancer::attach( Compound* compound )
{
    _clear();

    if( _compound )
    {
        _compound->removeListener( this );
        _compound = 0;
    }

    if( compound )
    {
        _compound = compound;
        compound->addListener( this );
    }
}

void LoadBalancer::_clear()
{
    delete _implementation;
    _implementation = 0;
}

void LoadBalancer::notifyChildAdded( Compound* compound, Compound* child )
{
    _clear();
}

void LoadBalancer::notifyChildRemove( Compound* compound, Compound* child )
{
    _clear();
}

void LoadBalancer::notifyUpdatePre( Compound* compound,
                                    const uint32_t frameNumber )
{
    EQASSERT( _compound );
    EQASSERT( _compound == compound );
    
    if( !_implementation )
    {
        switch( _mode )
        {
            case MODE_DB:
            case MODE_HORIZONTAL:
            case MODE_VERTICAL:
            case MODE_2D:
                _implementation = new TreeLoadBalancer( *this );
                break;

            case MODE_DPLEX:
                _implementation = new SmoothLoadBalancer( *this );
                break;
            case MODE_DFR:
                _implementation = new DFRLoadBalancer( *this );
                break;
            default:
                EQUNREACHABLE;
                return;
        }
    }

    EQLOG( LOG_LB ) << "Balance for frame " << frameNumber << endl;
    _implementation->update( frameNumber );
}

std::ostream& operator << ( std::ostream& os, 
                            const LoadBalancer::Mode mode )
{
    os << ( mode == LoadBalancer::MODE_2D         ? "2D" :
            mode == LoadBalancer::MODE_VERTICAL   ? "VERTICAL" :
            mode == LoadBalancer::MODE_HORIZONTAL ? "HORIZONTAL" :
            mode == LoadBalancer::MODE_DB         ? "DB" :
            mode == LoadBalancer::MODE_DFR        ? "DFR" :
            mode == LoadBalancer::MODE_DPLEX      ? "DPLEX" : "ERROR" );
    return os;
}

std::ostream& operator << ( std::ostream& os, const LoadBalancer* lb )
{
    if( !lb )
        return os;

    os << disableFlush
       << "loadBalancer " << endl
       << '{' << endl
       << "    mode " << lb->getMode() << endl;
  
    if( lb->getMode() == LoadBalancer::MODE_DFR )
        os << "    framerate " << lb->getFrameRate() << endl;

    if( lb->getDamping() >= 0.f )
        os << "    damping " << lb->getDamping() << endl;
    


    os << '}' << endl << enableFlush;
    return os;
}

}
}


/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "timeStamper.h"

#include <lunchbox/thread.h>
#include <lunchbox/sleep.h>

namespace massVolVis
{


class TimeStampTrigger : public lunchbox::Thread
{
public:

    // dalay time in ms
    TimeStampTrigger( const uint64_t delay, TimeStampSharedPtr timeStampPtr )
        : _delay(     delay     )
        , _timeStampPtr( timeStampPtr )
    {}

    virtual void run()
    {
        while( true )
        {
            _timeStampPtr->increment();
            lunchbox::sleep( _delay );
        }
    }

private:
    uint64_t           _delay;
    TimeStampSharedPtr _timeStampPtr;
};


TimeStamper::TimeStamper( TimeStampSharedPtr timeStamp, const uint64_t delay )
    : _timeStampPtr( timeStamp )
    , _triggerPtr( new TimeStampTrigger( delay, _timeStampPtr ))
{
    LBASSERT( _timeStampPtr );
    _triggerPtr->start();
}


TimeStamper::~TimeStamper()
{
    _triggerPtr->cancel();
    _triggerPtr->join();
}


}//namespace massVolVis


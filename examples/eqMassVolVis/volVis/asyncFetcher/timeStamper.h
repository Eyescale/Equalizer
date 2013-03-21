
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__TIME_STAMPER_H
#define MASS_VOL__TIME_STAMPER_H

#include <msv/types/nonCopyable.h>

#include "timeStamp.h"

#include <stdint.h>
#include <memory> // std::auto_ptr
#include <boost/shared_ptr.hpp>

namespace massVolVis
{

class TimeStampTrigger;

typedef boost::shared_ptr<TimeStamp> TimeStampSharedPtr;

/**
 *  Starts a separate thread that increments a value periodically.
 */
class TimeStamper : private NonCopyable
{
public:
    // delay is in ms (default triggering is 100 Hz)
    // requires that object exists
    explicit TimeStamper( TimeStampSharedPtr timeStamp, const uint64_t dalay = 10 );
    ~TimeStamper();

    void reset() { _timeStampPtr->reset(); }

    TimeStamp get() const { return *_timeStampPtr; }

private:
    TimeStampSharedPtr _timeStampPtr;

    std::auto_ptr<TimeStampTrigger> _triggerPtr;
};

}//namespace massVolVis

#endif //MASS_VOL__TIME_STAMPER_H


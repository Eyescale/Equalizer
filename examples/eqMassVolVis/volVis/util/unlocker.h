
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__UNLOCKER_H
#define MASS_VOL__UNLOCKER_H


#include <lunchbox/lock.h>

namespace massVolVis
{

/**
 *  Helper class that unlocks a lock after function exits.
 */
class Unlocker
{
public:
    explicit Unlocker( lunchbox::Lock& lock, const bool lockOnInit = true )
        : _lock( lock )
    {
        if( lockOnInit )
            _lock.trySet();
    }
    ~Unlocker()
    {
        _lock.unset();
    }
private:
    lunchbox::Lock& _lock;
};

}

#endif //MASS_VOL__RAM_POOL_H

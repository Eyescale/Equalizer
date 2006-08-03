
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_USERDATA_H
#define EQBASE_USERDATA_H

namespace eqBase
{
    /**
     * Base class providing set/get userdata on objects
     */
    template< typename T > class Userdata 
    {
    public:
        Userdata() : _userdata(0) {}
        virtual ~Userdata() {}

        T& getUserdata() { return _userdata; }
        const T& getUserdata() const { return _userdata; }
        void setUserdata( T& data ) { _userdata = data; }

    private:
        T _userdata;
    };
}

#endif //EQBASE_USERDATA_H

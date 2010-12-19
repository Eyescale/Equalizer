
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <eq/base/base.h>
#ifdef EQ_INFINIBAND

#ifndef CO_IBADAPTER_H
#define CO_IBADAPTER_H
#include <iba/ib_al.h>
#include <iba/ib_types.h>

namespace co
{
//
// Class for controlling an Infiniband Adapter
//
class IBAdapter
{
  public:
    IBAdapter()
        : _adapter( 0 )
        , _adapterAttr( 0 )
        , _accessLayer( 0 )
        , _protectionDomain( 0 ){}
    virtual ~IBAdapter(){ close(); }

    // open the channel adapter  and read specific attribute for build 
    // a protection domain 
    bool open();

    // close the channel adapter
    void close();
    // get a references handle to the opened channel adapter
    ib_ca_handle_t getHandle()  const { return _adapter;  }
    // get a references handle to the allocated protection domain
    ib_pd_handle_t getProtectionDomain() const { return _protectionDomain; }

    uint16_t getLid( const int ib_port ) const
            { return _adapterAttr->p_port_attr[ib_port-1].lid; } 

  private:

    ib_ca_handle_t  _adapter;
    ib_ca_attr_t*   _adapterAttr;
    ib_al_handle_t  _accessLayer;
    ib_pd_handle_t  _protectionDomain;

};
}
#endif //CO_IBADAPTER_H 
#endif //EQ_INFINIBAND

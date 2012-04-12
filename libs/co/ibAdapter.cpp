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

#include <eq/base/log.h>
#include "ibAdapter.h"
#ifdef EQ_INFINIBAND

namespace co
{

void IBAdapter::close()
{
    if ( _protectionDomain ) 
    {
        ib_api_status_t ibStatus = ib_dealloc_pd( _protectionDomain, 0 );
        _protectionDomain = 0;
    }

    if ( _adapterAttr )
        free( _adapterAttr );
    _adapterAttr = 0;

    if ( _adapter )
        ib_close_ca( _adapter, 0 );
    _adapter = 0;

    if ( _accessLayer )
        ib_close_al( _accessLayer );
    _accessLayer = 0;
}

bool IBAdapter::open( )
{
    
    ib_api_status_t ibStatus;

    // opens an instance of the access layer
    ibStatus = ib_open_al( &_accessLayer );
    if( ibStatus != IB_SUCCESS )
    {
        LBERROR << "Can't open AL instance !!!" << std::endl;
        return false;
    }
    
    // get a list of GUIDS for all channel adapter currently available in
    // the system
    // need to know the size
    size_t  guidCount;
    ibStatus = ib_get_ca_guids( _accessLayer, 0, &guidCount );
    if( ibStatus != IB_INSUFFICIENT_MEMORY )
    {
        LBERROR << "Can't get the Local CA Guids !!!" << std::endl;
        return false;
    }

    //If no CA's Present then return
    if( guidCount == 0 )
    {
        LBERROR << "No CA's Present !!!" << std::endl;
        return false;
    }

    // get a list of GUIDS for all channel adapter currently available in
    // the system
    ib_net64_t  *caGuidArray;
    caGuidArray = ( ib_net64_t* )malloc( sizeof( ib_net64_t ) * guidCount );
    ibStatus = ib_get_ca_guids( _accessLayer, caGuidArray, &guidCount );
    if( ibStatus != IB_SUCCESS )
    {
        LBERROR << "Can't get the Local CA Guids !!!" << std::endl;
        return false;
    }

    // Opens a channel adapter for additional access
    ibStatus = ib_open_ca( _accessLayer, caGuidArray[0] , 0, 0, &_adapter );
    if( ibStatus != IB_SUCCESS )
    {
        LBERROR << "Can't Open CA !!!" << std::endl;
        return false;
    }
    free( caGuidArray );

    /* Query the CA */
    uint32_t bsize = 0;
    // Queries the attributes of an opened channel adapter
    // need to know the size
    ibStatus = ib_query_ca( _adapter, 0, &bsize );
    if( ibStatus != IB_INSUFFICIENT_MEMORY )
    {
        LBERROR << "Can't Query CA !!!" << std::endl;
        return false;
    }

    // Queries the attributes of an opened channel adapter
    _adapterAttr = ( ib_ca_attr_t * )malloc( bsize );
    ibStatus = ib_query_ca( _adapter, _adapterAttr, &bsize );
    if( ibStatus != IB_SUCCESS )
    {
        LBERROR << "Can't Query CA !!!" << std::endl;
        return false;
    }

    // Allocates a protection domain on the specified channel adapter
    // IB_PDT_NORMAL : Protection domain for all non-aliased QPs.
    ibStatus = ib_alloc_pd( _adapter ,
                            IB_PDT_NORMAL, 
                            0,
                            &_protectionDomain  );

    if ( ibStatus != IB_SUCCESS ) 
    {
        LBERROR << "Can't Allocate Protection Domain !!!" << std::endl;
        return false;
    }

    return true;
}
}
#endif //EQ_INFINIBAND

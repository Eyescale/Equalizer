
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "worker.h"

#include "command.h"

namespace co
{
template< class Q > void WorkerThread< Q >::run()
{
    while( !stopRunning( ))
    {
        Command& command = *(_commands.pop( ));
        LBASSERT( command.isValid( ));

        if( !command( ))
        {
            EQABORT( "Error handling " << command );
        }
        command.release();

        while( _commands.isEmpty( ))
            if( !notifyIdle( )) // nothing to do
                break;
    }
 
    _commands.flush();
    LBINFO << "Leaving worker thread " << lunchbox::className( this ) << std::endl;
}

}


/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

using namespace eq::base;

namespace eq
{
namespace net
{
NamedPipeConnection::NamedPipeConnection( const ConnectionType type )
{
    
}

NamedPipeConnection::~NamedPipeConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool NamedPipeConnection::connect()
{
    return true;
}

void NamedPipeConnection::close()
{
    
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
void NamedPipeConnection::_initAIOAccept(){ /* NOP */ }
void NamedPipeConnection::_exitAIOAccept(){ /* NOP */ }
void NamedPipeConnection::_initAIORead(){ /* NOP */ }
void NamedPipeConnection::_exitAIORead(){ /* NOP */ }

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
void NamedPipeConnection::acceptNB(){ /* NOP */ }
 
ConnectionPtr NamedPipeConnection::acceptSync()
{
    return 0;
}
}
}

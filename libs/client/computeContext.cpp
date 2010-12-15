
/* Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "computeContext.h"

#include "pipe.h"

namespace eq
{

ComputeContext::ComputeContext( Pipe* parent )
        : _pipe( parent )
{
    EQASSERT( _pipe ); 
}

ComputeContext::~ComputeContext()
{
}

void ComputeContext::setError( const uint32_t error )
{
    _pipe->setError( error );
}

}

/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
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

#ifndef EQNBODY_PIPE_H
#define EQNBODY_PIPE_H

#include <eq/eq.h>
#include "SharedData.h"

namespace eqNbody
{	
    class Pipe : public eq::Pipe
    {
    public:
        Pipe( eq::Node* parent );

        SharedData& getSharedData() { return *_data; }
		
    protected:
        virtual ~Pipe() {}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

        virtual void frameStart( const uint32_t frameID, const uint32_t frameNumber );

    private:
		SharedData *_data;
    };
}

#endif // EQNBODY_PIPE_H

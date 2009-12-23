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

#ifndef EQNBODY_H
#define EQNBODY_H

#include <eq/eq.h>

#define NUM_BODIES	16384
#define MAX_NGPUS	32

namespace eqNbody
{
    class InitData;
	class Config;
	
    class Client : public eq::Client
    {
    public:
        Client( const InitData& initData );

        int init();
        int exit();
		
        void run();

    protected:
        virtual ~Client() {}
        virtual bool clientLoop();
        
    private:
        const InitData&	_initData;
		Config*			_config;
		eq::ServerPtr	_server;
    };
}

#endif // EQNBODY_H


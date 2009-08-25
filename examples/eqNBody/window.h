/*
 * Copyright (c) 2009, Philippe Robert <probert@eyescale.ch> 
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

#ifndef EQNBODY_WINDOW_H
#define EQNBODY_WINDOW_H

#include <eq/eq.h>

#include "configEvent.h"
#include "controller.h"
#include "dataProxy.h"
#include "client.h" // MAX_NGPUS

namespace eqNbody
{
	enum Mode
	{
		WINDOW_GL,
		WINDOW_CUDA,
		WINDOW_CUDA_GL,
	};
	
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) : eq::Window( parent ), _device(0) {}
		
		unsigned int getCUDADeviceID() { return _device; }
		unsigned int getMode() { return _mode; }
		
    protected:
        virtual ~Window() {}

		virtual bool configInit( const uint32_t initID );		
		virtual bool configInitCUDA();		
        virtual void swapBuffers();		
		
    private:
		int	_getMaxGflopsDeviceId();

		int		_device;
		Mode	_mode;
    };
}

#endif // EQNBODY_WINDOW_H

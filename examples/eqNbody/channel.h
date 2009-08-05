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

#ifndef EQNBODY_CHANNEL_H
#define EQNBODY_CHANNEL_H

#include <eq/eq.h>

#include "configEvent.h"
#include "controller.h"
#include "dataProxy.h"
#include "frameData.h"

namespace eqNbody
{
    class FrameData;
    class InitData;

    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent ) : eq::Channel( parent ) {}

    protected:
        virtual ~Channel() {}

        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );
		
	private:
		void _compute(const uint32_t frameID, const FrameData& fd);
		void _draw(const uint32_t frameID, const FrameData& fd);
		void _update();

        void _initLocalProxy();
        void _initCUDAController();

		void _initDataProxies(const FrameData& frameData);
		void _syncDataProxies(const FrameData& frameData);
		void _sendEvent(ConfigEvent::Type type, unsigned int version, unsigned int pid, const eq::Range& range);
		
		Controller		_controller;		
		DataProxy		_dataProxy[MAX_NGPUS];	
		unsigned int	_offset;
    };
}



#endif // EQ_PLY_CHANNEL_H


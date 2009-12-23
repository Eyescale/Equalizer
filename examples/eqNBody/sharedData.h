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


#ifndef EQNBODY_SHAREDDATA_H
#define EQNBODY_SHAREDDATA_H

#include <eq/eq.h>
#include <vector>

#include "frameData.h"
#include "configEvent.h"

namespace eqNbody
{
	class SharedDataProxy;
	class Config;
	class Controller;
	
    class SharedData
    {
    public:
		
        SharedData(Config *cfg);
		virtual ~SharedData();
								
		const FrameData& getFrameData() const { return _frameData; }
		
		void registerMemory( const eq::Range& range );
		void mapMemory();
		void syncMemory();
		void updateMemory( const eq::Range& range, Controller *controller );

		float* getPos() { return _frameData.getPos(); }
		float* getCol() { return _frameData.getCol(); }
		float* getVel() { return _frameData.getVel(); }

		float  getTimeStep() { return _frameData.getTimeStep(); }
		unsigned int getNumBytes() { return _frameData.getNumBytes(); }
		
		bool useStatistics() const { return _frameData.useStatistics(); }
		
    protected:
		
    private:   
		void _sendEvent(ConfigEvent::Type type, unsigned int version, unsigned int pid, const eq::Range& range);
		
		std::vector< SharedDataProxy* >	_proxies;
		FrameData					_frameData;
		Config*						_cfg;
    };
}

#endif // EQNBODY_SHAREDDATA_H
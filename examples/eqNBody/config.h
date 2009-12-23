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

#ifndef EQNBODY_CONFIG_H
#define EQNBODY_CONFIG_H

#include <eq/eq.h>

#include "initData.h"
#include "frameData.h"
#include "configEvent.h"

namespace eqNbody
{
    class Config : public eq::Config
    {
    public:
        Config( eq::base::RefPtr< eq::Server > parent );
				
        virtual bool init();
        virtual bool exit();
		
        virtual uint32_t startFrame();
		
        void setInitData( const InitData& data ) { _initData = data; }
        const InitData& getInitData() const { return _initData; }
		
        void mapData( const uint32_t initDataID );		
        void unmapData();
		
        virtual bool handleEvent( const eq::ConfigEvent* event );
        bool needsRedraw();
				
    protected:
        virtual ~Config() {}
		
    private:
        InitData	_initData;
        FrameData	_frameData;
        bool		_redraw;
				
		bool _readyToCommit();
		bool _handleKeyEvent( const eq::KeyEvent& event );

		void _updateSimulation();
		void _registerData(const ConfigEvent* event);
		void _updateData(const ConfigEvent* event);
        void _deregisterData();						
    };
}

#endif // EQ_PLY_CONFIG_H

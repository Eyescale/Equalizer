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

#ifndef EQNBODY_INITDATA_H
#define EQNBODY_INITDATA_H

#include <eq/eq.h>

namespace eqNbody
{
    class InitData : public eq::net::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const uint32_t id )   { _frameDataID = id; }
        uint32_t getFrameDataID() const   { return _frameDataID; }

		float getDamping() const { return _damping; }
		uint32_t getNumBodies() const { return _numBodies; }
		uint32_t getP() const { return _p; }
		uint32_t getQ() const { return _q; }
		
    protected:
        virtual void getInstanceData( eq::net::DataOStream& os );
        virtual void applyInstanceData( eq::net::DataIStream& is );

    private:
        uint32_t	_frameDataID;
		uint32_t	_numBodies;		// number of bodies in simulation
		uint32_t	_p;				// CUDA thread parameter p
		uint32_t	_q;				// CUDA thread parameter q
		float		_damping;		// damping factor
    };
}

#endif // EQNBODY_INITDATA_H


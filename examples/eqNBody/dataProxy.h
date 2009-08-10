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

#ifndef EQNBODY_DATAPROXY_H
#define EQNBODY_DATAPROXY_H

#include <eq/eq.h>

namespace eqNbody
{
    class DataProxy : public eq::Object
    {
    public:

        DataProxy();

		void init(const unsigned int offset, const unsigned int numBytes, float *pos, float *vel, float *col);
		void init(float *pos, float *vel, float *col);
		void exit();	
		
		void markDirty();

		unsigned int getOffset() const {return _offset;}
		unsigned int getNumBytes() const {return _numBytes;}
		
		float* getPosition() const {return _hPos;}
		float* getVelocity() const {return _hVel;}
		
    protected:
        virtual void serialize( eq::net::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_DATA   = eq::Object::DIRTY_CUSTOM << 0
        };
		
    private:        		
		unsigned int _offset;	// offset into the frameData's memory chunk
		unsigned int _numBytes;	// number of bytes to be written
		
		float*	_hPos;			// frameData's position data on the host
		float*	_hVel;			// frameData's velocity data on the host
		float*	_hCol;			// frameData's color data on the host
    };
}


#endif // EQNBODY_DATAPROXY_H


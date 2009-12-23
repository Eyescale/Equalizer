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

#ifndef EQNBODY_FRAMEDATA_H
#define EQNBODY_FRAMEDATA_H

#include <eq/eq.h>
#include "client.h" // MAX_NGPUS

namespace eqNbody
{	
	enum NBodyConfig
	{
		NBODY_CONFIG_RANDOM,
		NBODY_CONFIG_SHELL,
		NBODY_CONFIG_EXPAND,
		NBODY_NUM_CONFIGS
	};
	
    class FrameData : public eq::Object
    {
    public:

        FrameData();
        virtual ~FrameData();

		void init(unsigned int numBodies);
		void initHostData();
		void exit();		

		void updateParameters(NBodyConfig config, float clusterScale, float velocityScale, float ts);
		
        void toggleStatistics();
        bool useStatistics() const { return _statistics; }
		
		unsigned int getNumDataProxies( void ) const { return _numDataProxies; }
		unsigned int getProxyID( unsigned int ndx ) const { return _dataProxyID[ndx][0]; }
		unsigned int getProxyVersion( unsigned int ndx ) const { return _dataProxyID[ndx][1]; }
		unsigned int getVersionForProxyID( unsigned int pid ) const;

		void addProxyID( unsigned int pid, const float *range );
		void updateProxyID( unsigned int pid, unsigned int version, const float *range );
		
		virtual uint32_t commit();
		bool isReady();

		const float* getPosData() const {return _hPos;}
		const float* getVelData() const {return _hVel;}
		const float* getColData() const {return _hCol;}
		
		float getTimeStep() const { return _deltaTime; }
		float getClusterScale() const { return _clusterScale; }
		float getVelocityScale() const { return _velocityScale; }
		
		float* getPos() const { return _hPos; }
		float* getVel() const { return _hVel; }
		float* getCol() const { return _hCol; }
		
		unsigned int getNumBytes( void ) const { return 4*sizeof(float)*_numBodies; }
		unsigned int getNumBodies( void ) const { return _numBodies; }

    protected:
        virtual void serialize( eq::net::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_DATA		= eq::Object::DIRTY_CUSTOM << 0,
            DIRTY_PROXYDATA	= eq::Object::DIRTY_CUSTOM << 1,
            DIRTY_FLAGS		= eq::Object::DIRTY_CUSTOM << 2
        };
		
    private:        		
		void _randomizeData(NBodyConfig config);
		
        bool            _statistics;
        bool            _newParameters;

		unsigned int	_numDataProxies;			// total number of proxies
		unsigned int	_dataProxyID[MAX_NGPUS][2];	// ID, version
		float			_dataRanges[MAX_NGPUS];		// ranges covered by proxies

		unsigned int	_numBodies;		// number of bodies in the simulation
		float			_deltaTime;		// time step
		float			_clusterScale;	// scale of the cluster
		float			_velocityScale;	// velocity scaling
		
		float*			_hPos;			// initial position data on the host
		float*			_hVel;			// initial velocity data on the host
		float*			_hCol;			// initial color data
    };
}


#endif // EQNBODY_FRAMEDATA_H


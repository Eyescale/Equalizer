
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
    
    class FrameData : public eq::fabric::Serializable
    {
    public:

        FrameData();
        virtual ~FrameData();

        void init(unsigned int numBodies);
        void initHostData();
        void exit();        

        void updateParameters( NBodyConfig config, float clusterScale,
                               float velocityScale, float ts );
        
        void toggleStatistics();
        bool useStatistics() const { return _statistics; }
        
        unsigned int getNumDataProxies( void ) const { return _numDataProxies; }
        eq::uint128_t getProxyID( unsigned int ndx ) const
            { return _dataProxyID[ndx].identifier; }
        eq::uint128_t getProxyVersion( unsigned int ndx ) const
            { return _dataProxyID[ndx].version; }
        eq::uint128_t getVersionForProxyID( const eq::uint128_t& pid ) const;

        void addProxyID( const eq::uint128_t& pid, const float *range );
        void updateProxyID( const eq::uint128_t& pid,
                            const eq::uint128_t& version,
                            const float *range );
        
        virtual uint32_t commitNB( const uint32_t incarnation );
        bool isReady();

        const float* getPosData() const { return _hPos; }
        const float* getVelData() const { return _hVel; }
        const float* getColData() const { return _hCol; }
        
        float getTimeStep() const { return _deltaTime; }
        float getClusterScale() const { return _clusterScale; }
        float getVelocityScale() const { return _velocityScale; }
        
        float* getPos() const { return _hPos; }
        float* getVel() const { return _hVel; }
        float* getCol() const { return _hCol; }
        
        uint32_t getNumBytes( void ) const 
            { return 4 * sizeof(float) * _numBodies; }
        uint32_t getNumBodies( void ) const { return _numBodies; }

    protected:
        virtual void serialize( co::DataOStream& os, 
                                const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is, 
                                  const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_DATA      = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
            DIRTY_PROXYDATA = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
            DIRTY_FLAGS     = eq::fabric::Serializable::DIRTY_CUSTOM << 2
        };
        
    private:                
        void _randomizeData( NBodyConfig config );
        
        bool            _statistics;
        bool            _newParameters;

        uint32_t        _numDataProxies;          // total number of proxies
        co::ObjectVersion _dataProxyID[ MAX_NGPUS ];// ID, version
        float           _dataRanges[ MAX_NGPUS ]; // ranges covered by proxies

        uint32_t    _numBodies;     // number of bodies in the simulation
        float       _deltaTime;     // time step
        float       _clusterScale;  // scale of the cluster
        float       _velocityScale; // velocity scaling
        
        float*      _hPos;          // initial position data on the host
        float*      _hVel;          // initial velocity data on the host
        float*      _hCol;          // initial color data
    };
}

#endif // EQNBODY_FRAMEDATA_H



/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "frameData.h"
#include "nbody.h"

#include <cuda.h>

#if CUDART_VERSION >= 2020
# define ENABLE_HOSTALLOC
#endif

namespace eqNbody
{
FrameData::FrameData() : _statistics( true ) , _numDataProxies(0), _hPos(0)
                       , _hVel(0), _hCol(0)
{
    _numBodies      = 0;
    _deltaTime      = 0.0f;
    _clusterScale   = 0.0f;
    _velocityScale  = 0.0f;
    _newParameters  = false;
}

FrameData::~FrameData()
{
    _numDataProxies = 0;
}

void FrameData::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_DATA )
        if(_hPos && _hVel && _hCol)
            os << co::Array< float >( _hPos, _numBodies * 4 )
               << co::Array< float >( _hVel, _numBodies * 4 )
               << co::Array< float >( _hCol, _numBodies * 4 );

    if( dirtyBits & DIRTY_FLAGS )
        os << _statistics << _numBodies << _clusterScale << _velocityScale
           << _deltaTime << _newParameters;

    if( dirtyBits & DIRTY_PROXYDATA )
        os << _numDataProxies
           << co::Array< co::ObjectVersion >( _dataProxyID, MAX_NGPUS )
           << co::Array< float >( _dataRanges, MAX_NGPUS );
}

void FrameData::deserialize( co::DataIStream& is,
                             const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_DATA )
        if(_hPos && _hVel && _hCol)
            is >> co::Array< float >( _hPos, _numBodies*4 )
               >> co::Array< float >( _hVel, _numBodies*4 )
               >> co::Array< float >( _hCol, _numBodies*4 );

    if( dirtyBits & DIRTY_FLAGS )
        is >> _statistics >> _numBodies >> _clusterScale >> _velocityScale
           >> _deltaTime >> _newParameters;

    if( dirtyBits & DIRTY_PROXYDATA )
        is >> _numDataProxies
           >> co::Array< co::ObjectVersion >( _dataProxyID, MAX_NGPUS )
           >> co::Array< float >( _dataRanges, MAX_NGPUS );
}

void FrameData::addProxyID( const eq::uint128_t& pid, const eq::Range& range )
{
    LBASSERT(_numDataProxies < MAX_NGPUS);

    _dataProxyID[_numDataProxies].identifier = pid;
    _dataProxyID[_numDataProxies].version = co::VERSION_NONE;

    _dataRanges[_numDataProxies++] = (range.end - range.start);
    setDirty( DIRTY_PROXYDATA );
}

void FrameData::updateProxyID( const eq::uint128_t& pid,
                               const eq::uint128_t& version,
                               const eq::Range& range )
{
    // TODO: Better use a hash here!
    for(unsigned int i=0; i< _numDataProxies; i++)
    {
        if( pid == _dataProxyID[i].identifier )
        {
            _dataProxyID[i].version = version;
            _dataRanges[i] = (range.end - range.start);
            break;
        }
    }
    setDirty( DIRTY_PROXYDATA );
}

eq::uint128_t FrameData::commit()
{
    const eq::uint128_t v = co::Serializable::commit();
    for( unsigned int i = 0; i< _numDataProxies; ++i )
        _dataRanges[i] = 0.0f;

    return v;
}

bool FrameData::isReady()
{
    float length = 0.0f;

    for(unsigned int i=0; i<_numDataProxies; i++) {
        length += _dataRanges[i];
    }

    // Return true if range [0 1] is covered, otherwise false
    return (length == 1.0f) ? true : false;
}

eq::uint128_t FrameData::getVersionForProxyID( const eq::uint128_t& pid ) const
{
    // TODO: Better use a hash here!
    for( unsigned int i=0; i< _numDataProxies; ++i )
    {
        if( pid == getProxyID( i ))
            return getProxyVersion( i );
    }

    return 0;
}

void FrameData::toggleStatistics()
{
    _statistics = !_statistics;
    setDirty( DIRTY_FLAGS );
}

void FrameData::init(const unsigned int numBodies)
{
    _numBodies  = numBodies;

    setDirty( DIRTY_FLAGS );
}

void FrameData::initHostData()
{
#ifdef ENABLE_HOSTALLOC
    allocateHostArrays(&_hPos, &_hVel, &_hCol, _numBodies*4*sizeof(float));
#else
    _hPos       = new float[_numBodies*4];
    _hVel       = new float[_numBodies*4];
    _hCol       = new float[_numBodies*4];
#endif

    memset(_hPos, 0, _numBodies*4*sizeof(float));
    memset(_hVel, 0, _numBodies*4*sizeof(float));
    memset(_hCol, 0, _numBodies*4*sizeof(float));

    setDirty( DIRTY_DATA );
}

void FrameData::exit()
{
    _numDataProxies = 0;
    _numBodies      = 0;

#ifdef ENABLE_HOSTALLOC
    deleteHostArrays(_hPos, _hVel, _hCol);
#else
    delete [] _hPos;
    delete [] _hVel;
    delete [] _hCol;
#endif
}

void FrameData::updateParameters(NBodyConfig config, float clusterScale, float velocityScale, float ts)
{
    _clusterScale   = clusterScale;
    _velocityScale  = velocityScale;
    _deltaTime      = ts;
    _newParameters  = true;

    _randomizeData(config);

    setDirty( DIRTY_DATA );
    setDirty( DIRTY_FLAGS );
}

void FrameData::_randomizeData(NBodyConfig config)
{
    switch(config)
    {
      default:
      case NBODY_CONFIG_RANDOM:
      {
          float scale  = _clusterScale * std::max(1.0f, _numBodies / (1024.f));
          float vscale = _velocityScale * scale;

          int p = 0, v = 0;
          unsigned int i = 0;
          while (i < _numBodies)
          {
              eq::Vector3f point;
              //const int scale = 16;
              point.x() = rand() / (float) RAND_MAX * 2 - 1;
              point.y() = rand() / (float) RAND_MAX * 2 - 1;
              point.z() = rand() / (float) RAND_MAX * 2 - 1;
              float lenSqr = point.squared_length();
              if (lenSqr > 1)
                  continue;
              eq::Vector3f velocity;
              velocity.x() = rand() / (float) RAND_MAX * 2 - 1;
              velocity.y() = rand() / (float) RAND_MAX * 2 - 1;
              velocity.z() = rand() / (float) RAND_MAX * 2 - 1;
              lenSqr = velocity.squared_length();
              if (lenSqr > 1)
                  continue;

              _hPos[p++] = point.x() * scale; // pos.x
              _hPos[p++] = point.y() * scale; // pos.y
              _hPos[p++] = point.z() * scale; // pos.z
              _hPos[p++] = 1.0f; // mass

              _hVel[v++] = velocity.x() * vscale; // pos.x
              _hVel[v++] = velocity.y() * vscale; // pos.x
              _hVel[v++] = velocity.z() * vscale; // pos.x
              _hVel[v++] = 1.0f; // inverse mass

              i++;
          }
      }
      break;
      case NBODY_CONFIG_SHELL:
      {
          float scale = _clusterScale;
          float vscale = scale * _velocityScale;
          float inner = 2.5f * scale;
          float outer = 4.0f * scale;

          int p = 0, v=0;
          unsigned int i = 0;
          while (i < _numBodies)//for(int i=0; i < numBodies; i++)
          {
              float x, y, z;
              x = rand() / (float) RAND_MAX * 2 - 1;
              y = rand() / (float) RAND_MAX * 2 - 1;
              z = rand() / (float) RAND_MAX * 2 - 1;

              eq::Vector3f point( x, y, z );
              float len = point.normalize();
              if (len > 1)
                  continue;

              _hPos[p++] =  point.x() * (inner + (outer - inner) * rand() / (float) RAND_MAX);
              _hPos[p++] =  point.y() * (inner + (outer - inner) * rand() / (float) RAND_MAX);
              _hPos[p++] =  point.z() * (inner + (outer - inner) * rand() / (float) RAND_MAX);
              _hPos[p++] = 1.0f;

              x = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
              y = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
              z = 1.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
              eq::Vector3f axis( x, y, z );
              axis.normalize();

              if (1 - point.dot(axis) < 1e-6)
              {
                  axis.x() = point.y();
                  axis.y() = point.x();
                  axis.normalize();
              }
              //if (point.y < 0) axis = scalevec(axis, -1);
              eq::Vector3f vv( _hPos[4*i], _hPos[4*i+1], _hPos[4*i+2] );
              vv.cross( axis );
              _hVel[v++] = vv.x() * vscale;
              _hVel[v++] = vv.y() * vscale;
              _hVel[v++] = vv.z() * vscale;
              _hVel[v++] = 1.0f;

              i++;
          }
      }
      break;
      case NBODY_CONFIG_EXPAND:
      {
          float scale = _clusterScale * std::max(1.0f, _numBodies / (1024.f));
          float vscale = scale * _velocityScale;

          int p = 0, v = 0;
          for(unsigned int i=0; i < _numBodies;)
          {
              eq::Vector3f point;

              point.x() = rand() / (float) RAND_MAX * 2 - 1;
              point.y() = rand() / (float) RAND_MAX * 2 - 1;
              point.z() = rand() / (float) RAND_MAX * 2 - 1;

              float lenSqr = point.squared_length();
              if (lenSqr > 1)
                  continue;

              _hPos[p++] = point.x() * scale; // pos.x
              _hPos[p++] = point.y() * scale; // pos.y
              _hPos[p++] = point.z() * scale; // pos.z
              _hPos[p++] = 1.0f; // mass

              _hVel[v++] = point.x() * vscale; // pos.x
              _hVel[v++] = point.y() * vscale; // pos.x
              _hVel[v++] = point.z() * vscale; // pos.x
              _hVel[v++] = 1.0f; // inverse mass

              i++;
          }
      }
      break;
    }

    if (_hCol)
    {
        int v = 0;
        for(unsigned int i=0; i < _numBodies; i++)
        {
            _hCol[v++] = rand() / (float) RAND_MAX;
            _hCol[v++] = rand() / (float) RAND_MAX;
            _hCol[v++] = rand() / (float) RAND_MAX;
            _hCol[v++] = 1.0f;
        }
    }
}
}

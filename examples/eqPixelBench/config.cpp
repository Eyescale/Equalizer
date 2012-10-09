
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "config.h"
#include "configEvent.h"


namespace eqPixelBench
{
Config::Config( eq::ServerPtr parent )
        : eq::Config( parent )
        , _clock(0)
        , _modeTest( false )
{
}

Config::~Config()
{
    delete _clock;
    _clock = 0;
}

uint32_t Config::startFrame( const eq::uint128_t& frameID )
{
    if( !_clock )
        _clock = new lunchbox::Clock;

    _clock->reset();
    return eq::Config::startFrame( frameID );
}

bool Config::handleEvent( eq::EventICommand command )
{
    switch( command.getEventType( ))
    {
    case READBACK:
    case ASSEMBLE:
    case START_LATENCY:
    {
        switch( command.getEventType( ))
        {
        case READBACK:
            std::cout << "readback";
            break;
        case ASSEMBLE:
            std::cout << "assemble";
            break;
        case START_LATENCY:
        default:
            std::cout << "        ";
        }

        const float msec = command.get< float >();
        const std::string& name = command.get< std::string >();
        const eq::Vector2i area = command.get< eq::Vector2i >();
        const std::string& formatType = command.get< std::string >();
        const uint64_t dataSizeGPU = command.get< uint64_t >();
        const uint64_t dataSizeCPU = command.get< uint64_t >();

        std::cout << " \"" << name << "\" " << formatType
                  << std::string( 32-formatType.length(), ' ' ) << area.x()
                  << "x" << area.y() << ": ";

        if( msec < 0.0f )
            std::cout << "error 0x" << std::hex << static_cast< int >( -msec )
                      << std::dec;
        else
            std::cout << static_cast< uint32_t >( area.x() * area.y() /
                                                  msec / 1048.576f )
                      << "MPix/sec (" << msec << "ms, "
                      << unsigned(1000.0f / msec) << "FPS)";

        if( command.getEventType() == READBACK )
        {
            std::cout << area << "( size GPU : " << dataSizeGPU << " bytes ";
            std::cout << "/ size CPU : " << dataSizeCPU << " bytes ";
            std::cout << "/ time : " <<  msec << "ms )";
        }
        else if( command.getEventType() == ASSEMBLE )
        {
            std::cout << area << "( size CPU : " << dataSizeCPU << " bytes ";
            std::cout << "/ time : " <<  msec << "ms )";
        }
        return true;
    }

    default:
        return eq::Config::handleEvent( command );
    }
}
}

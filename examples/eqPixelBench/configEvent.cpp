
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "configEvent.h"

namespace eqPixelBench
{
std::ostream& printEvent( std::ostream& os, const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case READBACK:
            os  << "readback";
            break;

        case ASSEMBLE:
            os  << "assemble";
            break;
        case START_LATENCY:
            os  << "        ";
            break;
        default:
            os << event;
            return os;
    }

    // #145 Need copy of event for non-const get() of values :(
    eq::ConfigEvent myEvent( *event );

    const float msec = myEvent.get< float >();
    std::string name = myEvent.get< std::string >();
    eq::Vector2i area = myEvent.get< eq::Vector2i >();
    std::string formatType = myEvent.get< std::string >();
    uint64_t dataSizeGPU = myEvent.get< uint64_t >();
    uint64_t dataSizeCPU = myEvent.get< uint64_t >();

    os << " \"" << name << "\" " << formatType
       << std::string( 32-formatType.length(), ' ' ) << area.x()
       << "x" << area.y() << ": ";

    if( msec < 0.0f )
        os << "error 0x" << std::hex << static_cast< int >( -msec )
           << std::dec;
    else
        os << static_cast< uint32_t >( area.x() * area.y() / msec  / 1048.576f )
           << "MPix/sec (" << msec << "ms, " <<
            unsigned(1000.0f / msec) << "FPS)";

    if ( event->data.type == READBACK )
    {
        os << area << "( size GPU : " << dataSizeGPU << " bytes ";
        os << "/ size CPU : " << dataSizeCPU << " bytes ";
        os << "/ time : " <<  msec << "ms )";
    }
    else if ( event->data.type == ASSEMBLE )
    {
        os << area << "( size CPU : " << dataSizeCPU << " bytes ";
        os << "/ time : " <<  msec << "ms )";
    }

    return os;
}
}

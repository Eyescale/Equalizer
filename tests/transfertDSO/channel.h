
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PIXELBENCH_CHANNEL_H
#define EQ_PIXELBENCH_CHANNEL_H

#include <eq/eq.h>

namespace eqTransfertDSO 
{
struct ConfigEvent;

class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );
    virtual ~Channel() {}

protected:
    virtual void frameStart( const uint32_t frameID,
                             const uint32_t frameNumber );
    virtual void frameDraw( const uint32_t frameID );

    ConfigEvent _createConfigEvent();

private:
    void _draw( const uint32_t spin );
    void _testFormats();
    void _testTiledOperations();
    void _tiledOperations( uint32_t format, uint32_t type, uint32_t compressorName, 
                           ConfigEvent& eventColor );
    void _testDepthAssemble();
    void _depthAssemble( uint32_t format, uint32_t type, 
                              uint32_t compressorName, ConfigEvent& event );
private:
    eq::Frame _frame;
};
}

#endif // EQ_PIXELBENCH_CHANNEL_H


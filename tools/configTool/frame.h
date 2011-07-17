
/* Copyright (c)      2009, Maxim Makhinya
 *               2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CONFIGTOOL_FRAME_H
#define EQ_CONFIGTOOL_FRAME_H

#include <eq/server/frame.h>

#include <string>
#include <iostream>
#include <fstream>

class Frame
{
public:
    static inline eq::server::Frame* create( const char* name )
    {
        eq::server::Frame* frame = new eq::server::Frame;
        frame->setName( std::string( name ));
        return frame;
    }

    static inline eq::server::Frame* create( std::ostringstream& name )
    {
        return create( name.str().c_str( ));
    }

    static inline eq::server::Frame* create(      std::ostringstream& name,
          const eq::Viewport&       vp,
                bool                colorOnly = false )
    {
        eq::server::Frame* frame = create( name );
        frame->setViewport( vp );
        if( colorOnly )
            frame->setBuffers( eq::Frame::BUFFER_COLOR );
        return frame;
    }
};

#endif //EQ_CONFIGTOOL_FRAME_H

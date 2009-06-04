
/* Copyright (c) 2009, Makhinya Maxim
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

#include <server/frame.h>

#include <string>
#include <iostream>
#include <fstream>


class Frame : public eq::server::Frame
{
public:
    Frame( char* name ) : eq::server::Frame()
    {
        setName( std::string( name ));
    }
    Frame( std::ostringstream& name ) : eq::server::Frame()
    {
        setName( name.str( ));
    }
    Frame(      std::ostringstream& name,
          const eq::Viewport&       vp,
                bool                colorOnly = false ) : eq::server::Frame()
    {
        setName( name.str( ));
        setViewport( vp );
        if( colorOnly ) setBuffers( eq::Frame::BUFFER_COLOR );
    }
};

#endif //EQ_CONFIGTOOL_FRAME_H

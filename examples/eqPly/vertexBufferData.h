/*  
 *  Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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


#ifndef MESH_VERTEXBUFFERDATA_H
#define MESH_VERTEXBUFFERDATA_H


#include "typedefs.h"
#include <vector>
#include <fstream>


namespace mesh 
{
    
    
    /*  Holds the final kd-tree data, sorted and reindexed.  */
    class VertexBufferData
    {
    public:
        void clear()
        {
            vertices.clear();
            colors.clear();
            normals.clear();
            indices.clear();
        }
        
        /*  Write the vectors' sizes and contents to the given stream.  */
        void toStream( std::ostream& os )
        {
            writeVector( os, vertices );
            writeVector( os, colors );
            writeVector( os, normals );
            writeVector( os, indices );
        }
        
        /*  Read the vectors' sizes and contents from the given MMF address.  */
        void fromMemory( char** addr )
        {
            clear();
            readVector( addr, vertices );
            readVector( addr, colors );
            readVector( addr, normals );
            readVector( addr, indices );
        }
        
        std::vector< Vertex >       vertices;
        std::vector< Color >        colors;
        std::vector< Normal >       normals;
        std::vector< ShortIndex >   indices;
        
    private:
        /*  Helper function to write a vector to output stream.  */
        template< class T >
        void writeVector( std::ostream& os, std::vector< T >& v )
        {
            size_t length = v.size();
            os.write( reinterpret_cast< char* >( &length ), 
                      sizeof( size_t ) );
            if( length > 0 )
                os.write( reinterpret_cast< char* >( &v[0] ), 
                          length * sizeof( T ) );
        }
        
        /*  Helper function to read a vector from the MMF address.  */
        template< class T >
        void readVector( char** addr, std::vector< T >& v )
        {
            size_t length;
            memRead( reinterpret_cast< char* >( &length ), addr, 
                     sizeof( size_t ) );
            if( length > 0 )
            {
                v.resize( length );
                memRead( reinterpret_cast< char* >( &v[0] ), addr, 
                         length * sizeof( T ) );
            }
        }
    };
    
    
}


#endif // MESH_VERTEXBUFFERDATA_H

/*  
    vertexBufferRoot.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the VertexBufferRoot class.
*/


#ifndef MESH_VERTEXBUFFERROOT_H
#define MESH_VERTEXBUFFERROOT_H


#include "vertexBufferNode.h"
#include "vertexBufferData.h"


namespace mesh 
{
    
    
    /*  The class for kd-tree root nodes.  */
    class VertexBufferRoot : public VertexBufferNode
    {
    public:
        virtual void render( VertexBufferState& state ) const;
        
        void beginRendering( VertexBufferState& state ) const;
        void endRendering( VertexBufferState& state ) const;
        
        void setupTree( VertexData& data );
        bool writeToFile( const char* filename );
        bool readFromFile( const char* filename );
        bool hasColors() const { return _data.colors.size() > 0; }
        
    protected:
        virtual void toStream( std::ostream& os );
        virtual void fromMemory( char* start );
        
    private:
        bool constructFromPly( const char* filename );
        
        VertexBufferData _data;
    };
    
    
}


#endif // MESH_VERTEXBUFFERROOT_H

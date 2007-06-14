/*  
    vertexBufferBase.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the abstract VertexBufferBase class.
*/


#ifndef MESH_VERTEXBUFFERBASE_H
#define MESH_VERTEXBUFFERBASE_H


#include "typedefs.h"
#include <fstream>


namespace mesh 
{
    // defined elsewhere
    class VertexData;
    class VertexBufferData;
    class VertexBufferState;
        
    /*  The abstract base class for all kinds of kd-tree nodes.  */
    class VertexBufferBase
    {
    public:
        virtual void render( VertexBufferState& state, 
                             bool performCulling = true ) const = 0;
        
        const BoundingSphere& getBoundingSphere() const 
        {
            return _boundingSphere;
        }
        
        const GLfloat* getRange() const
        {
            return &_range[0];
        }
        
    protected:
        VertexBufferBase() : _boundingSphere( 0.0f ) 
        {
            _range[0] = 0.0f;
            _range[1] = 1.0f;
        }
        
        virtual ~VertexBufferBase() {};
        
        virtual void toStream( std::ostream& os )
        {
            os.write( reinterpret_cast< char* >( &_boundingSphere ), 
                      sizeof( BoundingSphere ) );
            os.write( reinterpret_cast< char* >( &_range ), 
                      sizeof( Range ) );
        }
        
        virtual void fromMemory( char** addr, VertexBufferData& globalData )
        {
            memRead( reinterpret_cast< char* >( &_boundingSphere ), addr, 
                     sizeof( BoundingSphere ) );
            memRead( reinterpret_cast< char* >( &_range ), addr, 
                     sizeof( Range ) );
        }
        
        virtual void setupTree( VertexData& data, const Index start,
                                const Index length, const Axis axis,
                                VertexBufferData& globalData ) = 0;
        
        virtual BoundingBox updateBoundingSphere() = 0;
        
        void calculateBoundingSphere( const BoundingBox& bBox )
        {
            _boundingSphere.center.x = ( bBox[0].x + bBox[1].x ) * 0.5f;
            _boundingSphere.center.y = ( bBox[0].y + bBox[1].y ) * 0.5f;
            _boundingSphere.center.z = ( bBox[0].z + bBox[1].z ) * 0.5f;
            Vertex delta;
            delta.x = _boundingSphere.center.x - bBox[0].x;
            delta.y = _boundingSphere.center.y - bBox[0].y;
            delta.z = _boundingSphere.center.z - bBox[0].z;
            _boundingSphere.radius = delta.length();
        }
        
        virtual void updateRange() = 0;
        
        BoundingSphere  _boundingSphere;
        Range           _range;
        
    private:
    };
    
    
}


#endif // MESH_VERTEXBUFFERBASE_H

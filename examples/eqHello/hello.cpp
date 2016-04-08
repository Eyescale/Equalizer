
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
 *
 * Equalizer 'Hello, World!' example. Shows the minimum Equalizer program,
 * rendering spinning quads around the origin.
 */

#include <seq/sequel.h>
#include <stdlib.h>

#include <eqHello/fragmentShader.glsl.h>
#include <eqHello/vertexShader.glsl.h>

namespace eqHello
{
class Renderer : public seq::Renderer
{
public:
    Renderer( seq::Application& application )
        : seq::Renderer( application )
        , _vertexArray( 0 )
        , _vertexBuffer( 0 )
        , _indexBuffer( 0 )
        , _colorBuffer( 0 )
        , _program( 0 )
        , _matrixUniform( 0 )
    {}
    virtual ~Renderer() {}

protected:
    void draw( co::Object* frameData ) final;
    bool initContext( co::Object* initData ) final;
    bool exitContext() final;

private:
    bool _loadShaders();
    void _setupCube();

    typedef vmml::vector< 3, GLushort > Vector3s;

    std::vector< seq::Vector3f > _vertices;
    std::vector< Vector3s > _triangles;
    std::vector< seq::Vector3f > _colors;

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLuint _colorBuffer;
    GLuint _program;
    GLuint _matrixUniform;
};

class Application : public seq::Application
{
    virtual ~Application() {}
public:
    virtual seq::Renderer* createRenderer() { return new Renderer( *this ); }
};
typedef lunchbox::RefPtr< Application > ApplicationPtr;
}

int main( const int argc, char** argv )
{
    eqHello::ApplicationPtr app = new eqHello::Application;

    if( app->init( argc, argv, 0 ) && app->run( 0 ) && app->exit( ))
        return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

void eqHello::Renderer::_setupCube()
{
    if( _vertexArray )
        return;

    _vertices = {
        // front
        seq::Vector3f(-0.5, -0.5,  0.5),
        seq::Vector3f( 0.5, -0.5,  0.5),
        seq::Vector3f( 0.5,  0.5,  0.5),
        seq::Vector3f(-0.5,  0.5,  0.5),
        // back
        seq::Vector3f(-0.5, -0.5, -0.5),
        seq::Vector3f( 0.5, -0.5, -0.5),
        seq::Vector3f( 0.5,  0.5, -0.5),
        seq::Vector3f(-0.5,  0.5, -0.5)
    };

    _triangles = {
        // front
        Vector3s(0, 1, 2),
        Vector3s(2, 3, 0),
        // top
        Vector3s(3, 2, 6),
        Vector3s(6, 7, 3),
        // back
        Vector3s(7, 6, 5),
        Vector3s(5, 4, 7),
        // bottom
        Vector3s(4, 5, 1),
        Vector3s(1, 0, 4),
        // left
        Vector3s(4, 0, 3),
        Vector3s(3, 7, 4),
        // right
        Vector3s(1, 5, 6),
        Vector3s(6, 2, 1)
    };

    _colors = {
        seq::Vector3f(0.5, 0.5, 0.5),
        seq::Vector3f(0.5, 0.5, 1.0),
        seq::Vector3f(0.5, 1.0, 0.5),
        seq::Vector3f(0.5, 1.0, 1.0),
        seq::Vector3f(1.0, 0.5, 0.5),
        seq::Vector3f(1.0, 0.5, 1.0),
        seq::Vector3f(1.0, 1.0, 0.5),
        seq::Vector3f(1.0, 1.0, 1.0)
    };

    seq::ObjectManager& om = getObjectManager();

    _vertexArray = om.newVertexArray( &_vertexArray );
    EQ_GL_CALL( glBindVertexArray( _vertexArray ));

    _vertexBuffer = om.newBuffer( &_vertexBuffer );
    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, _vertexBuffer ));
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER,
                              _vertices.size() * sizeof(seq::Vector3f),
                              _vertices.data(), GL_STATIC_DRAW ));
    EQ_GL_CALL( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 ));

    _colorBuffer = om.newBuffer( &_colorBuffer );
    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, _colorBuffer ));
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER,
                              _colors.size() * sizeof(seq::Vector3f),
                              _colors.data(), GL_STATIC_DRAW ));
    EQ_GL_CALL( glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 ));

    _indexBuffer = om.newBuffer( &_indexBuffer );
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ));
    EQ_GL_CALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                              _triangles.size() * sizeof(Vector3s),
                              _triangles.data(), GL_STATIC_DRAW ));

    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, 0 ));
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ));
    EQ_GL_CALL( glBindVertexArray( 0 ));
}

bool eqHello::Renderer::_loadShaders()
{
    seq::ObjectManager& om = getObjectManager();

    if( _program )
        return true;

    _program = om.newProgram( &_program );
    if( !seq::linkProgram( om.glewGetContext(), _program, vertexShader_glsl,
                           fragmentShader_glsl ))
    {
        return false;
    }

    EQ_GL_CALL( glUseProgram( _program ));
    _matrixUniform = glGetUniformLocation( _program, "MVP" );
    EQ_GL_CALL( glUseProgram( 0));
    return true;
}

bool eqHello::Renderer::initContext( co::Object* initData )
{
    if( !seq::Renderer::initContext( initData ))
        return false;

    if( !_loadShaders( ))
        return false;

    _setupCube();
    return true;
}

bool eqHello::Renderer::exitContext()
{
    seq::ObjectManager& om = getObjectManager();
    om.deleteProgram( &_program );
    om.deleteBuffer( &_vertexBuffer );
    om.deleteBuffer( &_colorBuffer );
    om.deleteBuffer( &_indexBuffer );
    om.deleteVertexArray( &_vertexArray );

    return seq::Renderer::exitContext();
}

/** The rendering routine, a.k.a., glutDisplayFunc() */
void eqHello::Renderer::draw( co::Object* /*frameData*/ )
{
    applyRenderContext(); // set up OpenGL State

    const seq::Matrix4f mvp = getFrustum().computePerspectiveMatrix() *
                              getViewMatrix() * getModelMatrix();

    EQ_GL_CALL( glUseProgram( _program ));
    EQ_GL_CALL( glUniformMatrix4fv( _matrixUniform, 1, GL_FALSE, mvp.data( )));
    EQ_GL_CALL( glBindVertexArray( _vertexArray ));
    EQ_GL_CALL( glEnableVertexAttribArray( 0 ));
    EQ_GL_CALL( glEnableVertexAttribArray( 1 ));
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ));

    EQ_GL_CALL( glDrawElements( GL_TRIANGLES, GLsizei(_triangles.size() * 3),
                                GL_UNSIGNED_SHORT, 0 ));

    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 1 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 0 ));
    EQ_GL_CALL( glBindVertexArray( 0 ));
    EQ_GL_CALL( glUseProgram( 0 ));
}


/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include <eq/eq.h>
#include <vertexBufferRoot.h>

namespace
{
static bool _isPlyfile( const std::string& filename )
{
    const size_t size = filename.length();
    if( size < 5 )
        return false;

    if( filename[size-4] != '.' || filename[size-3] != 'p' ||
        filename[size-2] != 'l' || filename[size-1] != 'y' )
    {
        return false;
    }
    return true;
}
}

int main( const int argc, char** argv )
{
    eq::Strings filenames;
    for( int i=1; i < argc; ++i )
        filenames.push_back( argv[i] );

    while( !filenames.empty( ))
    {
        const std::string filename = filenames.back();
        filenames.pop_back();
     
        if( _isPlyfile( filename ))
        {
            mesh::VertexBufferRoot* model = new mesh::VertexBufferRoot;
            if( !model->readFromFile( filename.c_str( )))
                EQWARN << "Can't load model: " << filename << std::endl;

            delete model;
        }
        else
        {
            const std::string basename = co::base::getFilename( filename );
            if( basename == "." || basename == ".." )
                continue;

            // recursively search directories
            const eq::Strings& subFiles =
                co::base::searchDirectory( filename, "*" );

            for(eq::StringsCIter i = subFiles.begin(); i != subFiles.end(); ++i)
                filenames.push_back( filename + '/' + *i );
        }
    }
}


/* Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010-2013, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef INITDATA_H
#define INITDATA_H

#include <eq/eq.h>

namespace osgScaleViewer
{
    /**
     * The init data holds all data which is needed during initalization.
     * It is sent by the server to all render clients.
     *
     * It holds the frame data ID, so all clients can sync the frame data
     * object. It also holds the model or image filename to load.
     */
    class InitData : public co::Object
    {
    public:
        InitData()
                : _modelFileName( "cow.osg" )
                , _imageFileName( "tests/compositor/Result_Alpha_color.rgb" ) {}
        virtual ~InitData() {}

        void setFrameDataID( const eq::uint128_t& id );
        const eq::uint128_t& getFrameDataID() const;

        /**
         * Sets the model filename.
         * @param filename the model filename.
         */
        void setModelFileName( const std::string& filename );

        /**
         * Gets the model filename.
         * @return the model filename.
         */
        std::string getModelFileName() const;

        /**
         * Sets the image filename.
         * @param filename the image filename.
         */
        void setImageFileName( const std::string& filename );

        /**
         * Gets the image filename.
         * @return the image filename.
         */
        std::string getImageFileName() const;

        /**
         * Gets the tracker port.
         * @return the tracker port.
         */
        const std::string getTrackerPort() const;

        /**
         * Parses the command line arguments and puts the things it can parse
         * into the init data.
         *
         * @return true if the command line was successfully parsed.
         */
        bool parseCommandLine( char **argv, int argc );

    protected:
        /** Reimplemented */
        virtual void getInstanceData( co::DataOStream& stream );

        /** Reimplemented */
        virtual void applyInstanceData( co::DataIStream& stream );

    private:
        /**
         * Parses the command line arguments.
         * @param argc the argument count.
         * @param argv the argument values.
         * @param param the parameter to parse.
         * @return the value of the param.
         */
        std::string _parseCommandLineParam( int argc, char** argv,
                                            std::string param );

        eq::uint128_t _frameDataID;

        std::string _modelFileName;
        std::string _imageFileName;

        std::string _trackerPort;
    };
}

#endif

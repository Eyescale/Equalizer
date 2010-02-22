
/* Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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
    class InitData : public eq::net::Object
    {
    public:
        InitData() 
                : _frameDataID( EQ_ID_INVALID )
                , _modelFileName( "cow.osg" )
                , _imageFileName( "tests/compositor/Result_Alpha_color.rgb" ) {}
        virtual ~InitData();

        void setFrameDataID( const uint32_t id );
        uint32_t getFrameDataID() const;

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
        virtual void getInstanceData( eq::net::DataOStream& stream );

        /** Reimplemented */
        virtual void applyInstanceData( eq::net::DataIStream& stream );

    private:
        /** 
         * Parses the command line arguments.
         * @param prefix the prefix to parse.
         * @return the value of the prefix.
         */
        std::string _parseCommandLinePrefix( char** argv, int argc,
                                             std::string prefix );

        uint32_t _frameDataID;

        std::string _modelFileName;
        std::string _imageFileName;

        std::string _trackerPort;
    };
}

#endif

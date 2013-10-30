#include <eq/client/fileFrameWriter.h>
#include <eq/client/channel.h>
#include <eq/client/image.h>
#include <eq/client/gl.h>
#include <eq/client/pipe.h>

#include <lunchbox/log.h>

#include <sstream>

eq::FileFrameWriter::FileFrameWriter( ) : _image( ) {}

eq::FileFrameWriter::~FileFrameWriter( ) {}

/*
 * This feature is considered for testing. For that reason any exception is
 * captured here. A message is displayed, but exceptions are not forwarded.
 */
void eq::FileFrameWriter::write( Channel *channel )
{
    _image.setAlphaUsage( true );
    _image.setQuality( Frame::BUFFER_COLOR, 1.0f );
    _image.setStorageType( Frame::TYPE_MEMORY );
    _image.setInternalFormat( Frame::BUFFER_COLOR, GL_RGBA );

    if( !_image.readback( Frame::BUFFER_COLOR ,
                          channel->getPixelViewport( ),
                          channel->getZoom( ),
                          channel->getObjectManager( ) ) )
        LBWARN << "Could not read frame buffer" << std::endl;
    else
    {
        const std::string& fileName = _buildFileName( channel );
        if( !_image.writeImage( fileName, Frame::BUFFER_COLOR ))
        {
            LBWARN << "Could not write file " << fileName << std::endl;
        }
    }

    _image.flush();
}

std::string eq::FileFrameWriter::_buildFileName( const eq::Channel * channel )
{
    std::string prefix = channel->getSAttribute(channel->SATTR_DUMP_IMAGE_PREFIX);
    std::stringstream name;
    name << prefix << channel->getPipe( )->getCurrentFrame( ) << ".rgb";
    return name.str();
}

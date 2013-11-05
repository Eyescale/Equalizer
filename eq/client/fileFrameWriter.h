#ifndef FILE_FRAME_WRITER_H
#define FILE_FRAME_WRITER_H

#include "eq/client/image.h"

#include <string>

namespace eq
{

/**
 * @brief The FileFrameWriter class
 *
 * It simply persists the contents of the COLOR buffer associated
 * with a channel to a file in the current working  directory. The name of
 * the file is frame<frameNumber>.rgb
 */
class FileFrameWriter
{
public:
    FileFrameWriter( );
    ~FileFrameWriter( );
    void write( eq::Channel * channel );

private:
    eq::Image _image;

    std::string _buildFileName( const eq::Channel * channel );
};

}

#endif

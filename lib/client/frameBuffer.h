
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAMEBUFFER_H
#define EQ_FRAMEBUFFER_H

#include <eq/client/frame.h>
#include <eq/client/viewport.h>
#include <eq/net/object.h>

namespace eqs
{
    class FrameBuffer;
}

namespace eq
{
    class Image;

    /**
     * A frame buffer holds multiple images and is used by frames.
     * It is not intended to be used directly by application code.
     */
    class FrameBuffer : public eqNet::Object
    {
    public:
        /** Instanciates a frame buffer. */
        FrameBuffer( const void* data, const uint64_t size );

        /** 
         * @name Data Access
         */
        //*{
        //*}

        /**
         * @name Operations
         */
        //*{

        /** Clear the frame by recycling the attached images. */
        void clear();

        /** Flush the frame by deleting all images. */
        void flush();

        /** 
         * Allocate and add a new image of a given format.
         * 
         * @param format the image format.
         * @return the image.
         */
        Image* newImage( const Frame::Format format );

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The newly read images are added to the frame, existing images are
         * retained.
         */
        void startReadback( const Frame::Format format );
        //*}

    protected:

        virtual ~FrameBuffer();

    private:
        enum FormatIndex
        {
            INDEX_COLOR,
            INDEX_DEPTH,
            INDEX_ALL
        };

        struct Data
        {
            Viewport      vp;
            Frame::Format format;
        }
            _data;

        friend class eqs::FrameBuffer;

        std::vector<Image*> _images[INDEX_ALL];
        std::vector<Image*> _imageCache[INDEX_ALL];

        void _clearImages( const FormatIndex index );
        void _flushImages( const FormatIndex index );

        FormatIndex _getIndexForFormat( const Frame::Format format );

        /* The command handlers. */
        //CommandResult _cmdEnter( Node* node, const Packet* pkg );
    };
}

#endif // EQ_FRAMEBUFFER_H



/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAMEBUFFER_H
#define EQ_FRAMEBUFFER_H

#include <eq/client/frame.h>

#include <eq/base/monitor.h>
#include <eq/net/object.h>

namespace eqs
{
    class FrameBuffer;
}

namespace eq
{
    class  Image;
    struct FrameBufferTransmitPacket;

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
         * The newly read images are added to the buffer, existing images are
         * retained.
         *
         * @param frame the corresponding output frame holder.
         */
        void startReadback( const Frame& frame );

        /** Synchronize the last image readback. */
        void syncReadback();

        /** 
         * Transmit the frame data to the specified node.
         *
         * Used internally after readback to push the image data to the input
         * frame nodes. Do not use directly.
         * 
         * @param toNode the receiving node.
         */        
        void transmit( eqBase::RefPtr<eqNet::Node> toNode );

        /** 
         * Set the frame buffer ready.
         * 
         * The frame buffer is automatically set ready by syncReadback
         * and upon processing of the transmit commands.
         */
        void setReady()        { _readyVersion = getVersion(); }

        /** @return true if the frame buffer is ready, false if not. */
        bool isReady() const   { return _readyVersion == getVersion(); }

        /** Wait for the frame buffer to become available. */
        void waitReady() const { _readyVersion.waitEQ( getVersion( )); }
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
            PixelViewport  pvp;
            vmml::Vector2i offset;
            Frame::Format  format;
        }
            _data;

        friend class eqs::FrameBuffer;

        std::vector<Image*> _images[INDEX_ALL];
        std::vector<Image*> _imageCache[INDEX_ALL];

        eqBase::Monitor<uint32_t> _readyVersion;

        void _clearImages( const FormatIndex index );
        void _flushImages( const FormatIndex index );

        FormatIndex _getIndexForFormat( const Frame::Format format );

        void _transmit( eqBase::RefPtr<eqNet::Node> toNode,
                        FrameBufferTransmitPacket& packet,
                        const Frame::Format format );

        /* The command handlers. */
        eqNet::CommandResult _cmdTransmit( eqNet::Command& command );
        eqNet::CommandResult _cmdReady( eqNet::Command& command );
    };
}

#endif // EQ_FRAMEBUFFER_H


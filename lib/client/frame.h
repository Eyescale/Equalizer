
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/client/eye.h>    // member enum
#include <eq/client/window.h> // nested ObjectManager type

#include <eq/base/monitor.h>
#include <eq/net/object.h>
#include <vmmlib/vector2.h>

namespace eqs
{
    class Frame;
}

namespace eq
{
    class FrameData;
    class Image;
    class Pipe;
    class Pixel;
    class Range;

    /**
     * A holder for a frame data and parameters.
     */
    class EQ_EXPORT Frame : public eqNet::Object
    {
    public:
        /** 
         * The buffer format defines which components of the frame are to
         * be used during recomposition.
         */
        enum Buffer
        {
            BUFFER_NONE      = EQ_BIT_NONE,
            BUFFER_UNDEFINED = EQ_BIT1,  //!< Inherit, only if no others are set
            BUFFER_COLOR     = EQ_BIT5,  //!< Use color images
            BUFFER_DEPTH     = EQ_BIT9,  //!< Use depth images
            BUFFER_ALL       = EQ_BIT_ALL
        };

        /** 
         * Constructs a new Frame.
         */
        Frame();
        virtual ~Frame();

        /**
         * @name Data Access
         */
        //*{
        const vmml::Vector2i& getOffset() const { return _data.offset; }
        
        /** The enabled frame buffer attachments. */
        uint32_t getBuffers() const;

        /** @return the database-range relative to the destination channel. */
        const Range& getRange() const;

        /** @return the pixel parameters relative to the destination channel. */
        const Pixel& getPixel() const;

        /** The images of this frame */
        const ImageVector& getImages() const;

        /** Set the data for this frame. */
        void setData( FrameData* data ) { _frameData = data; }
    
        const eqNet::ObjectVersion& getDataVersion( const Eye eye ) const
            { return _data.frameData[ eye ]; }
        //*}

        /**
         * @name Operations
         */
        //*{

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the frame, existing images are retained.
         *
         * @param glObjects the GL object manager for the current GL context.
         */
        void startReadback( Window::ObjectManager* glObjects );
        
        /** Synchronize the image readback. */
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
         * Test the readiness of the frame.
         * 
         * The readiness of the frame is automatically managed by the frame
         * buffer readback and transmit implementation.
         * 
         * @return true if the frame is ready, false if not. 
         */
        bool isReady() const;

        /** Wait for the frame to become available. */
        void waitReady() const;

        /** 
         * Add a listener which will be incremented when the frame is ready.
         * 
         * @param listener the listener.
         */
        void addListener( eqBase::Monitor<uint32_t>& listener );

        /** 
         * Remove a frame listener.
         * 
         * @param listener the listener.
         */
        void removeListener( eqBase::Monitor<uint32_t>& listener );
        //*}

        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         */
        void disableBuffer( const Buffer buffer );
        
    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }

    private:
        std::string _name;
        FrameData*  _frameData;

        /** The distributed data shared between eq::Frame and eqs::Frame. */
        friend class eqs::Frame;
        struct Data
        {
            Data() : offset( vmml::Vector2i::ZERO ), buffers( 0 ) {}

            vmml::Vector2i       offset;
            uint32_t             buffers;
            eqNet::ObjectVersion frameData[EYE_ALL];
        }
        _data;

    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const Frame::Buffer buffer );
};
#endif // EQ_FRAME_H

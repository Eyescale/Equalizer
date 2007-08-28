
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/client/eye.h> // member enum

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
            BUFFER_NONE      = 0,
            BUFFER_UNDEFINED = 0x1,      //!< Inherit, only if no others are set
            BUFFER_COLOR     = 0x10,     //!< Use color images
            BUFFER_DEPTH     = 0x10000,  //!< Use depth images
            BUFFER_ALL       = 0xfffffff
        };

        /** 
         * Constructs a new Frame.
         */
        Frame( Pipe* pipe );
        virtual ~Frame(){}

        /**
         * @name Data Access
         */
        //*{
        const vmml::Vector2i& getOffset() const { return _data.offset; }
        
        /** The database-range relativ to the destination channel. */
        const Range& getRange() const;

        /** The images of this frame data holder */
        const std::vector<Image*>& getImages() const;
        //*}

        /**
         * @name Operations
         */
        //*{

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the frame, existing images are retained.
         */
        void startReadback();
        
        /** Synchronize the image readback. */
        void syncReadback();

        /** 
         * Assemble all images according of the current frame data.
         */
        void startAssemble();
        
        /** Synchronize the image assembly. */
        void syncAssemble();

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

        /** Set by the channel */
        void setEyePass( const Eye eyePass ) { _eyePass = eyePass; }
        
        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         */
        void disableBuffer( const Frame::Buffer buffer );
        
    protected:
        virtual bool isStatic() const { return false; }

     private:
        std::string _name;
        Pipe*       _pipe;

        Eye _eyePass;

        /** All distributed data, shared between eq::Frame and eqs::Frame. */
        struct Data
        {
            vmml::Vector2i       offset;
            uint32_t             buffers;
            eqNet::ObjectVersion frameData[EYE_ALL];
        }
            _data;
        friend class eqs::Frame;

        FrameData* _getData() const;
    };
};
#endif // EQ_FRAME_H

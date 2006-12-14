
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAMEDATA_H
#define EQ_FRAMEDATA_H

#include <eq/client/frame.h>

#include <eq/base/monitor.h>
#include <eq/net/object.h>

namespace eqs
{
    class FrameData;
}

namespace eq
{
    class  Image;
    struct FrameDataTransmitPacket;

    /**
     * A frame data holds multiple images and is used by frames.
     * It is not intended to be used directly by application code.
     */
    class FrameData : public eqNet::Object
    {
    public:
        /** Instanciates a frame data. */
        FrameData( const void* data, const uint64_t size );

        /** 
         * @name Data Access
         */
        //*{
        //*}

        /**
         * @name Operations
         */
        //*{

        /** Flush the frame by deleting all images. */
        void flush();

        /** 
         * Allocate and add a new image.
         * 
         * @return the image.
         */
        Image* newImage();

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The newly read images are added to the data, existing images are
         * retained.
         *
         * @param frame the corresponding output frame holder.
         */
        void startReadback( const Frame& frame );

        /** Synchronize the last image readback. */
        void syncReadback();

        /** 
         * Assemble all images according of the current frame data.
         */
        void startAssemble( const Frame& frame );
        
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

        /** @return true if the frame data is ready, false if not. */
        bool isReady() const   { return _readyVersion == getVersion(); }

        /** Wait for the frame data to become available. */
        void waitReady() const { _readyVersion.waitEQ( getVersion( )); }

        /** 
         * Add a listener which will be incremented when the frame becomes
         * ready.
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

    protected:

        virtual ~FrameData();
        /** @sa eqNet::Object::unpack */
        virtual void unpack( const void* data, const uint64_t size );

    private:
        struct Data
        {
            PixelViewport  pvp;
            vmml::Vector2i offset;
            uint32_t       buffers;
        }
            _data;

        friend class eqs::FrameData;

        std::vector<Image*> _images;
        std::vector<Image*> _imageCache;

        struct ImageVersion
        {
            ImageVersion( Image* _image, const uint32_t _version )
                    : image( _image ), version( _version ) {}

            Image*   image;
            uint32_t version;
        };
        std::list<ImageVersion> _pendingImages;

        /** Data ready monitor synchronization primitive. */
        eqBase::Monitor<uint32_t> _readyVersion;

        /** External monitors for readyness synchronization. */
        std::vector< eqBase::Monitor<uint32_t>* > _listeners;
        eqBase::Lock                              _listenersMutex;
        /** Clear the frame by recycling the attached images. */
        void _clear();

        /** Allocate or reuse a new image. */
        Image* _allocImage();

        /** 
         * Set the frame data ready.
         * 
         * The frame data is automatically set ready by syncReadback
         * and upon processing of the transmit commands.
         */
        void _setReady();

        void _transmit( eqBase::RefPtr<eqNet::Node> toNode,
                        FrameDataTransmitPacket& packet,
                        const Frame::Buffer buffers );

        /* The command handlers. */
        eqNet::CommandResult _cmdTransmit( eqNet::Command& command );
        eqNet::CommandResult _cmdReady( eqNet::Command& command );
    };
    std::ostream& operator << ( std::ostream& os, const FrameData* data );
}

#endif // EQ_FRAMEDATA_H


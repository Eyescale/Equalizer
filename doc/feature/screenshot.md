Screenshot
============

This specifies the creation of screenshots in Equalizer

## API

    class View
    {
        void setRecording( Frame::Buffer buffers );
        void handleEvent( ScreenshotEvent event )
        {
            _screenshot[ event.frameID ].composite( event.vp,
                                                    event.imageData );
            if( _screenshot[ event.frameID ].isComplete( ))
                _screenshots.push( _screenshot[ event.frameID ]);
            // pop if queue "full" (latency+1)
        };

        Frame,frameID popRecordedImage(); // blocks potentially
    };

    struct ScreenshotEvent // client -> app
    {
        Viewport vp; // relative to View
        uint32_t frameID;
        ImageData imageData;
    };

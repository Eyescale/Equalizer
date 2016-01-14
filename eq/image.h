
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Enrique <egparedes@ifi.uzh.ch>
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

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/frame.h>         // for Frame::Buffer enum
#include <eq/types.h>

namespace eq
{
namespace detail { class Image; }

/**
 * A holder for pixel data.
 *
 * An image holds color and depth information for one rectangular region.
 */
class Image
{
public:
    /** Construct a new Image. @version 1.0 */
    EQ_API Image();

    /** Destruct the Image. @version 1.0 */
    EQ_API virtual ~Image();

    /** @name Image parameters */
    //@{
    /**
     * Set the internal format for the given buffer.
     *
     * The internal format descibes the format of the pixel data source,
     * typically an OpenGL frame buffer or texture. The internal formats used by
     * Equalizer use the same numerical value as their OpenGL counterpart.
     *
     * @param buffer the frame buffer attachment.
     * @param internalFormat the internal format.
     * @version 1.0
     */
    EQ_API void setInternalFormat( const Frame::Buffer buffer,
                                   const uint32_t internalFormat );

    /** @return the internal format of the pixel data. @version 1.0 */
    EQ_API uint32_t getInternalFormat( const Frame::Buffer buffer )const;

    /**
     * Get the external format of the given buffer.
     *
     * The external format describes the layout of the pixel data in main
     * memory. It is determined by the plugin used during download. The external
     * format also determines the pixel size and alpha availability.
     *
     * @param buffer the frame buffer attachment.
     * @return the external format of the pixel data.
     * @version 1.0
     */
    EQ_API uint32_t getExternalFormat( const Frame::Buffer buffer ) const;

    /**
     * Get the size, in bytes, of one pixel in the external pixel data.
     *
     * @param buffer the frame buffer attachment.
     * @sa getExternalFormat()
     * @version 1.0
     */
    EQ_API uint32_t getPixelSize( const Frame::Buffer buffer ) const;

    /**
     * @return true if the image has a color buffer with alpha values.
     * @sa getExternalFormat()
     * @version 1.0
     */
    EQ_API bool hasAlpha() const;

    /**
     * Set the frame pixel storage type.
     *
     * Images of storage type TYPE_MEMORY are read back from the frame buffer
     * into main memory using a transfer plugin. The data can be accessed
     * through the PixelData.
     *
     * Image of storage type TYPE_TEXTURE read frame buffer data into a texture,
     * which can be accessed using getTexture().
     * @version 1.0
     */
    EQ_API void setStorageType( const Frame::Type type );

    /** @return the pixel data storage type. @version 1.0 */
    EQ_API Frame::Type getStorageType() const;

    /**
     * Set the internal pixel viewport of the image.
     *
     * The image pixel data and textures will be invalidated. The pixel data
     * describes the size of the image on the destination (GPU). Each downloaded
     * buffer has its own size, which is potentially different from the image
     * PVP.
     *
     * @param pvp the pixel viewport.
     * @version 1.0
     */
    EQ_API void setPixelViewport( const PixelViewport& pvp );

    /** @return the internal pixel viewport. @version 1.0 */
    EQ_API const PixelViewport& getPixelViewport() const;

    /** Sets the zoom factor to be used for compositing. */
    EQ_API void setZoom( const Zoom& zoom );

    /** @return zoom factor to be used for compositing. */
    EQ_API const Zoom& getZoom() const;

    /** Set the render context producing this image. */
    EQ_API void setContext( const RenderContext& context );

    /** @return the rendering context which created this image, or a default. */
    EQ_API const RenderContext& getContext() const;

    /**
     * Set a compressor to be used during transmission of the image.
     *
     * The default compressor is EQ_COMPRESSOR_AUTO which selects the most
     * suitable compressor based on the current image and buffer parameters.
     *
     * @param buffer the frame buffer attachment.
     * @param name the compressor name
     */
    EQ_API void useCompressor( Frame::Buffer buffer, uint32_t name );

    /**
     * Reset the image to its default state.
     *
     * This method does not free allocated memory or plugins. Invalidates all
     * pixel data.
     *
     * @sa flush()
     * @version 1.0
     */
    EQ_API void reset();

    /** Free all cached data of this image. @version 1.0 */
    EQ_API void flush();

    /**
     * Delete all OpenGL objects allocated from the given object manager.
     *
     * Requires the appropriate OpenGL context to be current.
     * @version 1.3.2
     */
    EQ_API void deleteGLObjects( util::ObjectManager& om );

    /**
     * Deallocate all transfer and compression plugins.
     *
     * Requires the OpenGL context used during readback to be current.
     * @version 1.3.2
     */
    EQ_API void resetPlugins();
    //@}

    /** @name Pixel Data Access */
    //@{
    /** @return a pointer to the raw pixel data. @version 1.0 */
    EQ_API const uint8_t* getPixelPointer( const Frame::Buffer buffer )
        const;

    /** @return a pointer to the raw pixel data. @version 1.0 */
    EQ_API uint8_t* getPixelPointer( const Frame::Buffer buffer );

    /** @return the total size of the pixel data in bytes. @version 1.0 */
    EQ_API uint32_t getPixelDataSize( const Frame::Buffer buffer ) const;

    /** @return the pixel data. @version 1.0 */
    EQ_API const PixelData& getPixelData( const Frame::Buffer ) const;

    /** @return the pixel data, compressing it if needed. @version 1.0 */
    EQ_API const PixelData& compressPixelData( const Frame::Buffer );

    /**
     * @return true if the image has valid pixel data for the buffer.
     * @version 1.0
     */
    EQ_API bool hasPixelData( const Frame::Buffer buffer ) const;

    /**
     * @return true if an async readback for a buffer is in progress.
     * @version 1.3.2
     */
    EQ_API bool hasAsyncReadback( const Frame::Buffer buffer ) const;

    /**
     * @return true if an async readback for any buffer is in progress.
     * @version 1.3.2
     */
    EQ_API bool hasAsyncReadback() const;

    /**
     * Clear and validate an image buffer.
     *
     * RGBA and BGRA buffers are initialized with (0,0,0,255).
     * DEPTH_UNSIGNED_INT buffers are initialized with 255. All other buffers
     * are zero-initialized. Validates the buffer.
     *
     * @param buffer the image buffer to clear.
     * @version 1.0
     */
    EQ_API void clearPixelData( const Frame::Buffer buffer );

    /** Allocate an image buffer without initialization. @version 1.0 */
    EQ_API void validatePixelData( const Frame::Buffer buffer );

    /**
     * Set the pixel data of the given image buffer.
     *
     * Previous data for the buffer is overwritten. Validates the
     * buffer. Depending on the given PixelData parameters, the pixel data is
     * copied, decompressed or cleared.
     *
     * @param buffer the image buffer to set.
     * @param data the pixel data.
     * @version 1.0
     */
    EQ_API void setPixelData( const Frame::Buffer buffer,
                              const PixelData& data );

    /**
     * Set alpha data preservation during download and compression.
     * @version 1.0
     */
    EQ_API void setAlphaUsage( const bool enabled );

    /** @return true if alpha data can not be ignored. @version 1.0 */
    EQ_API bool getAlphaUsage() const;

    /**
     * Set the minimum quality after a full download-compression path.
     *
     * The automatic selection of a download and compression plugin will never
     * always choose plugins which maintain at least the given quality. A
     * quality of 1.0 enables a lossless transmission path, and a quality of 0.0
     * disables all quality quarantees.
     *
     * @param buffer the frame buffer attachment.
     * @param quality the minimum quality to maintain.
     * @version 1.0
     */
    EQ_API void setQuality( const Frame::Buffer buffer,
                            const float quality );

    /** @return the minimum quality. @version 1.0 */
    EQ_API float getQuality( const Frame::Buffer buffer ) const;
    //@}

    /** @name Texture Data Access */
    //@{
    /** Get the texture of this image. @version 1.0 */
    EQ_API const util::Texture& getTexture( const Frame::Buffer buffer )
        const;

    /**
     * @return true if the image has texture data for the buffer.
     * @version 1.0
     */
    EQ_API bool hasTextureData( const Frame::Buffer buffer ) const;
    //@}

    /** @name Operations */
    //@{
    /**
     * Start reading back an image from the frame buffer.
     *
     * @param buffers bit-wise combination of the Frame::Buffer components.
     * @param pvp the area of the frame buffer wrt the drawable.
     * @param context the render context producing the pixel data.
     * @param zoom the scale factor to apply during readback.
     * @param glObjects the GL object manager for the current GL context.
     * @return true when the operation requires a finishReadback().
     * @version 1.3.2
     */
    EQ_API bool startReadback( const uint32_t buffers, const PixelViewport& pvp,
                               const RenderContext& context, const Zoom& zoom,
                               util::ObjectManager& glObjects );

    /** @internal Start reading back data from a texture. */
    bool startReadback( const Frame::Buffer buffer,
                        const util::Texture* texture,
                        const GLEWContext* glewContext );

    /**
     * Finish an asynchronous readback.
     *
     * @param glewContext the OpenGL function table.
     * @version 1.7.1
     */
    EQ_API void finishReadback( const GLEWContext* glewContext );

    /**
     * Upload this image to the frame buffer or a texture.
     *
     * If a texture is given, the upload is performed to it. Otherwise the pixel
     * data is uploaded to the frame buffer. The texture will be initialized
     * using the parameters corresponding to the requested buffer.
     *
     * @param buffer the buffer type.
     * @param texture the target texture, or 0 for frame buffer upload.
     * @param position the destination offset wrt current GL viewport.
     * @param glObjects the OpenGL object manager for the current context.
     * @return true on success, false on error.
     * @version 1.0
     */
    EQ_API bool upload( const Frame::Buffer buffer, util::Texture* texture,
                        const Vector2i& position,
                        util::ObjectManager& glObjects ) const;

    /**
     * Write the pixel data as rgb image file.
     *
     * Since version 1.9 (if build with OpenSceneGraph) this function can
     * write images according to supported plugins, see
     * http://trac.openscenegraph.org/projects/osg/wiki/Support/UserGuides/Plugins
     * @version 1.0
     */
    EQ_API bool writeImage( const std::string& filename,
                            const Frame::Buffer buffer ) const;

    /** Write all valid pixel data as separate images. @version 1.0 */
    EQ_API bool writeImages( const std::string& filenameTemplate ) const;

    /** Read pixel data from an uncompressed rgb image file. @version 1.0 */
    EQ_API bool readImage( const std::string& filename,
                           const Frame::Buffer buffer );

    /** @internal Set image offset after readback to correct position. */
    void setOffset( int32_t x, int32_t y );
    //@}

    /** @name Internal */
    //@{
    /**
     * @internal
     * @return the list of possible compressors for the given buffer.
     */
    EQ_API std::vector< uint32_t >
    findCompressors( const Frame::Buffer buffer ) const;

    /**
     * @internal
     * @return a list of possible up/downloaders for the given buffer.
     */
    EQ_API std::vector< uint32_t >
    findTransferers( const Frame::Buffer buffer, const GLEWContext* gl )
        const;

    /** @internal Re-allocate, if needed, a compressor instance. */
    EQ_API bool allocCompressor( const Frame::Buffer buffer,
                                 const uint32_t name );

    /** @internal Re-allocate, if needed, a downloader instance. */
    EQ_API bool allocDownloader( const Frame::Buffer buffer,
                                 const uint32_t name,
                                 const GLEWContext* glewContext );

    /** @internal */
    EQ_API uint32_t getDownloaderName( const Frame::Buffer buffer ) const;
    //@}

private:
    Image( const Image& ) = delete;
    Image& operator=( const Image& )  = delete;
    detail::Image* const _impl;

    /** @return a unique key for the frame buffer attachment. */
    const void* _getBufferKey( const Frame::Buffer buffer ) const;

    /** @return a unique key for the frame buffer attachment. */
    const void* _getCompressorKey( const Frame::Buffer buffer ) const;

    /**
     * Set the type of the pixel data in main memory for the given buffer.
     *
     * Invalidates the pixel data.
     *
     * @param buffer the buffer type.
     * @param externalFormat the type of the pixel.
     * @param pixelSize the size of one pixel in bytes.
     * @param hasAlpha true if the pzixel data contains an alpha channel.
     */
    void _setExternalFormat( const Frame::Buffer buffer,
                             const uint32_t externalFormat,
                             const uint32_t pixelSize,
                             const bool hasAlpha );

    bool _readback( const Frame::Buffer buffer, const Zoom& zoom,
                    util::ObjectManager& glObjects );

    bool _startReadback( const Frame::Buffer buffer, const Zoom& zoom,
                         util::ObjectManager& glObjects );

    void _finishReadback( const Frame::Buffer buffer, const GLEWContext* );
    bool _readbackZoom( const Frame::Buffer buffer, util::ObjectManager& om );
    bool _writeImage( const std::string& filename, const Frame::Buffer buffer,
                      const unsigned char* data ) const;
};
};
#endif // EQ_IMAGE_H

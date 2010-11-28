
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQFABRIC_WINDOW_H
#define EQFABRIC_WINDOW_H

#include <eq/fabric/object.h>        // base class

#include <eq/fabric/drawableConfig.h> // enum
#include <eq/fabric/paths.h>
#include <eq/fabric/pixelViewport.h>
#include <eq/fabric/viewport.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
    class Window;
namespace server 
{ 
    class Window;
}
namespace fabric
{
    /** Base data transport class for windows. @sa eq::Window */
    template< class P, class W, class C > class Window : public Object
    {
    public:
        /** A vector of pointers to channels. @version 1.0 */
        typedef std::vector< C* >  Channels; 
        /** The Window visitor type. @version 1.0 */
        typedef ElementVisitor< W, LeafVisitor< C > > Visitor;

        /** @name Data Access */
        //@{
        /** @internal Initialize this window (calls virtual methods). */
        void init();

        /** @return the parent Pipe of this window. @version 1.0 */
        const P* getPipe() const { return _pipe; }
        /** @return the parent Pipe of this window. @version 1.0 */
        P* getPipe() { return _pipe; }

        /** @return a vector of all child channels of this window.  */
        const Channels& getChannels() const { return _channels; }

        /** @return the window's drawable configuration. @version 1.0 */
        const DrawableConfig& getDrawableConfig() const
            { return _data.drawableConfig; }

        /**
         * @return the window's pixel viewport wrt the parent pipe.
         * @version 1.0 */
        const PixelViewport& getPixelViewport() const { return _data.pvp; }

        /**
         * @return the window's fractional viewport wrt the parent pipe.
         * @version 1.0
         */
        const Viewport& getViewport() const { return _data.vp; }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport of the window and its channels
         * accordingly.
         * 
         * @param pvp the viewport in pixels.
         * @version 1.0
         */
        EQFABRIC_EXPORT virtual void setPixelViewport(const PixelViewport& pvp);

        /** 
         * Set the window's viewport wrt its parent pipe.
         * 
         * Updates the fractional pixel viewport of the window and its channels
         * accordingly.
         *
         * @param vp the fractional viewport.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setViewport( const eq::fabric::Viewport& vp );

        /** @return true if a viewport was specified last. @version 1.0 */
        bool hasFixedViewport( ) const { return _data.fixedVP; }

        /** @internal Notify this window that the viewport has changed. */
        void notifyViewportChanged();

        /** 
         * Traverse this window and all children using a window visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor ) const;

        //@}

        /** @name Attributes */
        //@{
        /** 
         * Window attributes.
         *
         * Most of these attributes are used by the SystemWindow implementation
         * to configure the window during configInit(). An SystemWindow
         * implementation might not respect all attributes, e.g.,
         * IATTR_HINT_SWAPSYNC is not implemented by the GLXWindow. Please refer
         * to the Programming Guide for details.  @version 1.0
         */
        enum IAttribute
        {
            // Note: also update string array initialization in window.ipp
            IATTR_HINT_STEREO,           //!< Active stereo
            IATTR_HINT_DOUBLEBUFFER,     //!< Front and back buffer
            IATTR_HINT_FULLSCREEN,       //!< Fullscreen drawable
            IATTR_HINT_DECORATION,       //!< Window decorations
            IATTR_HINT_SWAPSYNC,         //!< Swap sync on vertical retrace
            IATTR_HINT_DRAWABLE,         //!< Window, pbuffer or FBO
            IATTR_HINT_STATISTICS,       //!< Statistics gathering hint
            IATTR_HINT_SCREENSAVER,      //!< Screensaver (de)activation (WGL)
            IATTR_HINT_AFFINITY,         //!< affinity useability (WGL)
            IATTR_PLANES_COLOR,          //!< No of per-component color planes
            IATTR_PLANES_ALPHA,          //!< No of alpha planes
            IATTR_PLANES_DEPTH,          //!< No of z-buffer planes
            IATTR_PLANES_STENCIL,        //!< No of stencil planes
            IATTR_PLANES_ACCUM,          //!< No of accumulation buffer planes
            IATTR_PLANES_ACCUM_ALPHA,    //!< No of alpha accum buffer planes
            IATTR_PLANES_SAMPLES,        //!< No of multisample (AA) planes
            IATTR_LAST,
            IATTR_ALL = IATTR_LAST + 5
        };

        /** Set a window attribute. @version 1.0 */
        void setIAttribute( const IAttribute attr,
                                      const int32_t value )
            { _data.iAttributes[attr] = value; }

        /** @return the value of a window attribute. @version 1.0 */
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _data.iAttributes[attr]; }

        /** @internal @return the name of a window attribute. */
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                      const IAttribute attr );
        //@}

        /** @internal @return the index path to this window. */
        EQFABRIC_EXPORT WindowPath getPath() const;

        EQFABRIC_EXPORT virtual void backup(); //!< @internal
        EQFABRIC_EXPORT virtual void restore(); //!< @internal
        void create( C** channel ); //!< @internal
        void release( C* channel ); //!< @internal
        virtual void output( std::ostream& ) const {} //!< @internal
        //@}

    protected: 
        /** @internal Construct a new window. */
        Window( P* parent );

        EQFABRIC_EXPORT virtual ~Window(); //!< @internal

        virtual void attachToSession( const base::UUID& id,
                                      const uint32_t instanceID,
                                      net::Session* session ); //!< @internal

        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );
        
        EQFABRIC_EXPORT virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_EXPORT virtual void setDirty( const uint64_t bits );

        /** @internal */
        void _setDrawableConfig( const DrawableConfig& drawableConfig );

        /** @internal */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        C* _findChannel( const base::UUID& id ); //!< @internal

        enum DirtyBits
        {
            DIRTY_ATTRIBUTES      = Object::DIRTY_CUSTOM << 0,
            DIRTY_CHANNELS        = Object::DIRTY_CUSTOM << 1,
            DIRTY_VIEWPORT        = Object::DIRTY_CUSTOM << 2,
            DIRTY_DRAWABLECONFIG  = Object::DIRTY_CUSTOM << 3,
            DIRTY_WINDOW_BITS =
                DIRTY_ATTRIBUTES | DIRTY_CHANNELS | DIRTY_VIEWPORT |
                DIRTY_DRAWABLECONFIG | DIRTY_OBJECT_BITS
        };

        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_WINDOW_BITS; }

    private:
        /** The parent pipe. */
        P* const _pipe;

        /** The channels of this window. */
        Channels _channels;

        struct BackupData
        {
            BackupData();

            /** Integer attributes. */
            int32_t iAttributes[ IATTR_ALL ];

            /** Drawable characteristics of this window */
            DrawableConfig drawableConfig;

            /** The absolute size and position of the window. */
            PixelViewport pvp;
        
            /** The fractional size and position of the window. */
            Viewport vp;

            /** true if the pixel viewport is mutable, false if the viewport 
                is immutable */
            bool fixedVP;
        }
            _data, _backup;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        friend class Channel< W, C >;
        /** Add a new channel to this window. */
        void _addChannel( C* channel );

        /** Remove a channel from this window. */
        EQFABRIC_EXPORT bool _removeChannel( C* channel );

        EQFABRIC_EXPORT virtual uint32_t commitNB(); //!< @internal
        bool _mapNodeObjects() { return _pipe->_mapNodeObjects(); }

        typedef net::CommandFunc< Window< P, W, C > > CmdFunc;
        bool _cmdNewChannel( net::Command& command );
        bool _cmdNewChannelReply( net::Command& command );
    };

    template< class P, class W, class C > EQFABRIC_EXPORT
    std::ostream& operator << ( std::ostream& os,
                                const Window< P, W, C >& window );
}
}

#endif // EQFABRIC_WINDOW_H


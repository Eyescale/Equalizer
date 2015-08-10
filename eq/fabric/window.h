
/* Copyright (c) 2010-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include <eq/fabric/windowSettings.h>

namespace eq
{
namespace fabric
{
    /** Base data transport class for windows. @sa eq::Window */
    template< class P, class W, class C, class Settings = WindowSettings >
    class Window : public Object
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
        EQFABRIC_INL const PixelViewport& getPixelViewport() const;

        /**
         * @return the window's fractional viewport wrt the parent pipe.
         * @version 1.0
         */
        const Viewport& getViewport() const { return _data.vp; }

        /** Set the window's name/caption. @version 1.7.2 */
        EQFABRIC_INL void setName( const std::string& name ) final;

        /**
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport of the window and its channels
         * accordingly.
         *
         * @param pvp the viewport in pixels.
         * @version 1.0
         */
        EQFABRIC_INL virtual void setPixelViewport( const PixelViewport& pvp );

        /**
         * Set the window's viewport wrt its parent pipe.
         *
         * Updates the fractional pixel viewport of the window and its channels
         * accordingly.
         *
         * @param vp the fractional viewport.
         * @version 1.0
         */
        EQFABRIC_INL void setViewport( const Viewport& vp );

        /** @return true if a viewport was specified last. @version 1.0 */
        bool hasFixedViewport( ) const { return _data.fixedVP; }

        /** @internal Notify this window that the viewport has changed. */
        virtual void notifyViewportChanged();

        /**
         * Perform a depth-first traversal of this window.
         *
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_INL VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_INL VisitorResult accept( Visitor& visitor ) const;

        //@}

        /** @name Attributes */
        //@{
        /** Set the settings of this window. @version 1.9 */
        EQFABRIC_INL void setSettings( const Settings& settings );

        /** @return the settings of this window. @version 1.7.2 */
        EQFABRIC_INL const Settings& getSettings() const;

        /** Set a window attribute. @version 1.0 */
        EQFABRIC_INL void setIAttribute( const WindowSettings::IAttribute attr,
                                         const int32_t value );

        /** @return the value of a window attribute. @version 1.0 */
        EQFABRIC_INL int32_t
        getIAttribute( const WindowSettings::IAttribute attr ) const;

        /** @internal @return the name of a window attribute. */
        EQFABRIC_INL static const std::string&
        getIAttributeString( const WindowSettings::IAttribute attr );
        //@}

        /** @internal @return the index path to this window. */
        EQFABRIC_INL WindowPath getPath() const;

        /** @name internal */
        //@{
        EQFABRIC_INL virtual void backup(); //!< @internal
        EQFABRIC_INL virtual void restore(); //!< @internal
        void create( C** channel ); //!< @internal
        void release( C* channel ); //!< @internal
        virtual void output( std::ostream& ) const {} //!< @internal
        /** @internal */
        EQFABRIC_INL virtual uint128_t commit( const uint32_t incarnation =
                                               CO_COMMIT_NEXT );
        //@}

    protected:
        /** @internal Construct a new window. */
        explicit Window( P* parent );

        EQFABRIC_INL virtual ~Window(); //!< @internal
        /** @internal */
        virtual void attach( const uint128_t& id,
                             const uint32_t instanceID );

        /** @internal */
        EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_INL virtual void deserialize( co::DataIStream& is,
                                                  const uint64_t dirtyBits );

        EQFABRIC_INL virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_INL virtual void setDirty( const uint64_t bits );

        /** @internal */
        void _setDrawableConfig( const DrawableConfig& drawableConfig );

        /** @internal */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        C* _findChannel( const uint128_t& id ); //!< @internal

        /** @internal */
        enum DirtyBits
        {
            DIRTY_SETTINGS        = Object::DIRTY_CUSTOM << 0,
            DIRTY_CHANNELS        = Object::DIRTY_CUSTOM << 1,
            DIRTY_VIEWPORT        = Object::DIRTY_CUSTOM << 2,
            DIRTY_DRAWABLECONFIG  = Object::DIRTY_CUSTOM << 3,
            DIRTY_WINDOW_BITS =
                DIRTY_SETTINGS | DIRTY_CHANNELS | DIRTY_VIEWPORT |
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

            /** Window settings. */
            Settings windowSettings;

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

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        friend class Channel< W, C >;
        /** Add a new channel to this window. */
        void _addChannel( C* channel );

        /** Remove a channel from this window. */
        EQFABRIC_INL bool _removeChannel( C* channel );

        /** @internal */
        bool _mapNodeObjects() { return _pipe->_mapNodeObjects(); }

        typedef co::CommandFunc< Window< P, W, C, Settings > > CmdFunc;
        bool _cmdNewChannel( co::ICommand& command );
        bool _cmdNewChannelReply( co::ICommand& command );
    };

    template< class P, class W, class C, class Settings > EQFABRIC_INL
    std::ostream& operator << ( std::ostream& os,
                                const Window< P, W, C, Settings >& window );
}
}

#endif // EQFABRIC_WINDOW_H

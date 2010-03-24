
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
#include <eq/fabric/pixelViewport.h>
#include <eq/fabric/viewport.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
namespace fabric
{
    template< typename T, typename W > class Channel;
    template< typename T, typename C  > class ElementVisitor;
    template< typename T > class LeafVisitor;

    template< typename P, typename W, typename C > class Window : public Object
    {
    public:

        /** A vector of pointers to Channel */
        typedef std::vector< C* >  ChannelVector; 
        typedef ElementVisitor< W, LeafVisitor< C > > Visitor;

        /** @name Data Access */
        //@{
        /** @return the Pipe of this window. */
        const P* getPipe() const { return _pipe; }
        /** @return the Pipe of this window. */
        P*       getPipe()       { return _pipe; }

        /**  @return a vector of all channels of this window.  */
        const ChannelVector& getChannels() const { return _channels; }

        /** 
         * Return the set of tasks this window's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return the tasks.
         */
        uint32_t getTasks() const { return _tasks; }

        const DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

        /** @return the window's pixel viewport */
        EQFABRIC_EXPORT const PixelViewport& getPixelViewport() const { return _pvp; }

        /** @return the window's fractional viewport. */
        const Viewport& getViewport() const { return _vp; }

        /** 
         * Traverse this window and all children using a window visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor ) const;

        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in window.cpp
        /** 
         * Window attributes.
         *
         * Most of these attributes are used by the OSWindow implementation to
         * configure the window during configInit(). An OSWindow implementation
         * might not respect all attributes, e.g., IATTR_HINT_SWAPSYNC is not
         * implemented by the GLXWindow.
         */
        enum IAttribute
        {
            IATTR_HINT_STEREO,           //!< Active stereo
            IATTR_HINT_DOUBLEBUFFER,     //!< Front and back buffer
            IATTR_HINT_FULLSCREEN,       //!< Fullscreen drawable
            IATTR_HINT_DECORATION,       //!< Window decorations
            IATTR_HINT_SWAPSYNC,         //!< Swap sync on vertical retrace
            IATTR_HINT_DRAWABLE,         //!< Window, pbuffer or FBO
            IATTR_HINT_STATISTICS,       //!< Statistics gathering hint
            IATTR_HINT_SCREENSAVER,      //!< Screensaver (de)activation (WGL)
            IATTR_PLANES_COLOR,          //!< No of per-component color planes
            IATTR_PLANES_ALPHA,          //!< No of alpha planes
            IATTR_PLANES_DEPTH,          //!< No of z-buffer planes
            IATTR_PLANES_STENCIL,        //!< No of stencil planes
            IATTR_PLANES_ACCUM,          //!< No of accumulation buffer planes
            IATTR_PLANES_ACCUM_ALPHA,    //!< No of alpha accum buffer planes
            IATTR_PLANES_SAMPLES,        //!< No of multisample (AA) planes
            IATTR_FILL1,                 //!< Reserved for future extensions
            IATTR_FILL2,                 //!< Reserved for future extensions
            IATTR_ALL
        };

        /** Set a window attribute. */
        EQFABRIC_EXPORT void setIAttribute( const IAttribute attr,
                                      const int32_t value )
            { _iAttributes[attr] = value; }

        /** @return the value of a window attribute. */
        EQFABRIC_EXPORT int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }

        /** @return the name of a window attribute. */
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                      const IAttribute attr );
        //@}

        /** @name Error Information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within configInit().
         *
         * @param message the error message.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setErrorMessage( const std::string& message );

        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected: 

        /** Construct a new window. */
        Window( P* parent );

        /** Constructs a new deep copy of a window. */
        Window( const W& from, P* pipe );

        /** The absolute size and position of the window. */
        PixelViewport _pvp;
        
        /** The fractional size and position of the window. */
        Viewport _vp;
        
        friend class Channel< C, W >;

        /**
         * @name Data Access
         */
        //@{        
        /** Add a new channel to this window. */
        void _addChannel( C* channel );

        /** Remove a channel from this window. */
        bool _removeChannel( C* channel );

        C* _findChannel( const uint32_t id );

        /**  @return a vector of all channels of this window.  */
        ChannelVector& _getChannels() { return _channels; }

        void _setTasks( uint32_t tasks ) { _tasks = tasks; }

        void _setDrawableConfig( const DrawableConfig drawableConfig )
            { _drawableConfig = drawableConfig; }

        DrawableConfig& _getDrawableConfig()
            { return _drawableConfig; }
        //@}

        void _setPipe( P* pipe ){ _pipe = pipe; }

    private:

        /** Integer attributes. */
        int32_t _iAttributes[ IATTR_ALL ];
        
        /** The reason for the last error. */
        std::string _error;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The channels of this window. */
        ChannelVector     _channels;

        /** Drawable characteristics of this window */
        DrawableConfig _drawableConfig;

        /** The parent pipe. */
        P* _pipe;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };
}
}

#endif // EQFABRIC_WINDOW_H


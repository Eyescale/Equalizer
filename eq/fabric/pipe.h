
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_PIPE_H
#define EQFABRIC_PIPE_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/pixelViewport.h> // property
#include <eq/fabric/types.h>

namespace eq
{
namespace fabric
{
/** Base data transport class for pipes. @sa eq::Pipe */
template< class N, class P, class W, class V > class Pipe : public Object
{
public:
    /** A vector of pointers to windows. @version 1.0 */
    typedef std::vector< W* >  Windows;

    /** @name Data Access */
    //@{
    /** @return the parent node of this pipe. @version 1.0 */
    N*       getNode()       { return _node; }
    /** @return the parent node of this pipe. @version 1.0 */
    const N* getNode() const { return _node; }

    /** @return the vector of child windows. @version 1.0 */
    const Windows& getWindows() const { return _windows; }

    /**
     * Returns the port number of this pipe.
     *
     * The port number identifies the X server for systems using the X11/GLX
     * window system, i.e., the :<strong>&lt;port&gt;</strong>.&lt;screen&gt; of
     * the DISPLAY name. It currently has no meaning on all other systems.
     *
     * @return the port number of this pipe, or LB_UNDEFINED_UINT32.
     * @version 1.0
     */
    uint32_t getPort() const { return _port; }

    EQFABRIC_INL void setPort( const uint32_t port ); //!< @internal

    /**
     * Returns the device number of this pipe.
     *
     * The device number identifies the X screen for systems using the X11/GLX
     * window system, or the number of the virtual screen for the AGL window
     * system. On Windows systems it identifies the graphics adapter. Normally
     * the device identifies a GPU.
     *
     * @return the device number of this pipe, or LB_UNDEFINED_UINT32.
     * @version 1.0
     */
    uint32_t getDevice() const { return _device; }

    EQFABRIC_INL void setDevice( const uint32_t device ); //!< @internal

    /** @return the pixel viewport. @version 1.0 */
    const PixelViewport& getPixelViewport() const { return _data.pvp; }

    /**
     * Set the pipe's pixel viewport.
     *
     * If invalid, the OSPipe has to set it to the device viewport during
     * configInit().
     *
     * @param pvp the viewport in pixels.
     * @version 1.0
     */
    EQFABRIC_INL void setPixelViewport( const PixelViewport& pvp );

    /** @internal Notify this pipe that the viewport has changed. */
    void notifyPixelViewportChanged();

    /** @internal @return the index path to this pipe. */
    EQFABRIC_INL PipePath getPath() const;

    /**
     * Perform a depth-first traversal of this pipe.
     *
     * @param visitor the visitor.
     * @return the result of the visitor traversal.
     * @version 1.0
     */
    EQFABRIC_INL VisitorResult accept( V& visitor );

    /** Const-version of accept(). @version 1.0 */
    EQFABRIC_INL VisitorResult accept( V& visitor ) const;
    //@}

    /** @name Attributes */
    //@{
    /** Pipe attributes. @version 1.0 */
    enum IAttribute
    {
        // Note: also update string array initialization in pipe.cpp
        IATTR_HINT_THREAD,   //!< Execute tasks in separate thread (default)
        IATTR_HINT_AFFINITY, //!< Bind render thread to subset of cores
        IATTR_HINT_CUDA_GL_INTEROP, //!< Configure CUDA context
        IATTR_LAST,
        IATTR_ALL = IATTR_LAST + 5
    };

    /** @internal Set a pipe attribute. */
    EQFABRIC_INL void setIAttribute( const IAttribute attr,
                                     const int32_t value );

    /** @return the value of a pipe integer attribute. @version 1.0 */
    int32_t getIAttribute( const IAttribute attr ) const
    { return _iAttributes[attr]; }

    /** @internal @return true if tasks are executed in a separate thread.*/
    bool isThreaded() const
    { return (getIAttribute( IATTR_HINT_THREAD ) == 1 ); }

    /** @internal @return the name of a pipe attribute. */
    EQFABRIC_INL static const std::string& getIAttributeString(
        const IAttribute attr );
    //@}

    /** @name internal */
    //@{
    EQFABRIC_INL virtual void backup(); //!< @internal
    EQFABRIC_INL virtual void restore(); //!< @internal
    void create( W** window ); //!< @internal
    void release( W* window ); //!< @internal
    virtual void output( std::ostream& ) const {} //!< @internal
    /** @internal */
    EQFABRIC_INL virtual uint128_t commit( const uint32_t incarnation =
                                           CO_COMMIT_NEXT );
    //@}

protected:
    /** @internal Construct a new pipe. */
    explicit Pipe( N* parent );
    EQFABRIC_INL virtual ~Pipe( ); //!< @internal

    virtual void attach( const uint128_t& id,
                         const uint32_t instanceID ); //!< @internal
    /** @internal */
    EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                         const uint64_t dirtyBits );
    /** @internal */
    EQFABRIC_INL virtual void deserialize( co::DataIStream& is,
                                           const uint64_t dirtyBits );

    EQFABRIC_INL virtual void notifyDetach(); //!< @internal

    /** @internal @sa Serializable::setDirty() */
    EQFABRIC_INL virtual void setDirty( const uint64_t bits );

    /** @internal */
    virtual ChangeType getChangeType() const { return UNBUFFERED; }

    W* _findWindow( const uint128_t& id ); //!< @internal

    enum DirtyBits
    {
        DIRTY_ATTRIBUTES      = Object::DIRTY_CUSTOM << 0,
        DIRTY_WINDOWS         = Object::DIRTY_CUSTOM << 1,
        DIRTY_PIXELVIEWPORT   = Object::DIRTY_CUSTOM << 2,
        DIRTY_MEMBER          = Object::DIRTY_CUSTOM << 3,
        DIRTY_PIPE_BITS =
        DIRTY_ATTRIBUTES | DIRTY_WINDOWS | DIRTY_PIXELVIEWPORT |
        DIRTY_MEMBER | DIRTY_OBJECT_BITS
    };

    /** @internal @return the bits to be re-committed by the master. */
    virtual uint64_t getRedistributableBits() const
        { return DIRTY_PIPE_BITS; }

private:
    /** The parent node. */
    N* const _node;

    /** The list of windows. */
    Windows _windows;

    /** Integer attributes. */
    int32_t _iAttributes[IATTR_ALL];

    /** The display (GLX) or ignored (Win32, AGL). */
    uint32_t _port;

    /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
    uint32_t _device;

    struct BackupData
    {
        /** The size (and location) of the pipe. */
        PixelViewport pvp;
    }
        _data, _backup;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    void _addWindow( W* window );
    EQFABRIC_INL bool _removeWindow( W* window );
    template< class, class, class, class > friend class Window;

    /** @internal */
    bool _mapNodeObjects() { return _node->_mapNodeObjects(); }

    typedef co::CommandFunc< Pipe< N, P, W, V > > CmdFunc;
    bool _cmdNewWindow( co::ICommand& command );
    bool _cmdNewWindowReply( co::ICommand& command );
};

template< class N, class P, class W, class V > EQFABRIC_INL
std::ostream& operator << ( std::ostream&, const Pipe< N, P, W, V >& );
}
}

#endif // EQFABRIC_PIPE_H


/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_CANVAS_H
#define EQFABRIC_CANVAS_H

#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h>  // enum
#include <eq/fabric/frustum.h>        // base class
#include <eq/fabric/object.h>         // base class

#include <string>

namespace eq
{
namespace fabric
{
    struct CanvasPath;

    /** A canvas represents a logical 2D projection surface. */
    template< class CFG, class C, class S, class L >
    class Canvas : public Object, public Frustum
    {
    public:
        typedef std::vector< S* > Segments; //!< A vector of segments
        typedef std::vector< L* > Layouts; //!< A vector of layouts
        /** A Canvas visitor */
        typedef ElementVisitor< C, LeafVisitor< S > > Visitor; 
        
        /** @name Data Access */
        //@{
        /** @return the parent config. @version 1.0 */
        CFG*       getConfig()       { return _config; }
        /** @return the parent config. @version 1.0 */
        const CFG* getConfig() const { return _config; }

        /** @return the index of the active layout. @version 1.0 */
        uint32_t getActiveLayoutIndex() const { return _data.activeLayout; }

        /** @return the active layout. @version 1.0 */
        EQFABRIC_EXPORT const L* getActiveLayout() const;

        /** @return the vector of child segments. @version 1.0 */
        const Segments& getSegments() const { return _segments; }        

        /** Find the first segment of a given name. @internal */
        S* findSegment( const std::string& name );

        /** @return the vector of possible layouts. @version 1.0 */
        const Layouts& getLayouts() const { return _layouts; }        

        /** Add a new allowed layout to this canvas, can be 0. @internal */
        EQFABRIC_EXPORT void addLayout( L* layout );

        /** @sa Frustum::setWall() */
        EQFABRIC_EXPORT virtual void setWall( const Wall& wall );
        
        /** @sa Frustum::setProjection() */
        EQFABRIC_EXPORT virtual void setProjection( const Projection& );

        /** @sa Frustum::unsetFrustum() */
        EQFABRIC_EXPORT virtual void unsetFrustum();
        //@}

        /** @name Operations */
        //@{
        /** Activate the given layout on this canvas. @version 1.0 */
        EQFABRIC_EXPORT virtual void useLayout( const uint32_t index );

        /** 
         * Traverse this canvas and all children using a canvas visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor ) const;

        EQFABRIC_EXPORT virtual void backup(); //!< @internal
        EQFABRIC_EXPORT virtual void restore(); //!< @internal

        void create( S** segment ); //!< @internal
        void release( S* segment ); //!< @internal
        //@}

    protected:
        /** Construct a new Canvas. @internal */
        EQFABRIC_EXPORT Canvas( CFG* config );

        /** Destruct this canvas. @internal */
        EQFABRIC_EXPORT virtual ~Canvas();

        EQFABRIC_EXPORT virtual void attachToSession( const uint32_t id,
                                      const uint32_t instanceID,
                                      net::Session* session ); //!< @internal

        /** @sa Frustum::serialize. @internal */
        EQFABRIC_EXPORT void serialize( net::DataOStream& os, 
                                        const uint64_t dirtyBits );
        /** @sa Frustum::deserialize. @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        EQFABRIC_EXPORT virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_EXPORT virtual void setDirty( const uint64_t bits );

        /** @internal */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }
        /** @internal */
        virtual void activateLayout( const uint32_t index ) { /* NOP */ }

    private:
        /** The parent config. */
        CFG* const _config;

        struct BackupData
        {
            BackupData() : activeLayout( 0 ) {}

            /** The currently active layout on this canvas. */
            uint32_t activeLayout;
        }
            _data, _backup;

        /** Allowed layouts on this canvas. */
        Layouts _layouts;

        /** Child segments on this canvas. */
        Segments _segments;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        enum DirtyBits
        {
            DIRTY_LAYOUT    = Object::DIRTY_CUSTOM << 0,
            DIRTY_SEGMENTS  = Object::DIRTY_CUSTOM << 1,
            DIRTY_LAYOUTS   = Object::DIRTY_CUSTOM << 2,
            DIRTY_FRUSTUM   = Object::DIRTY_CUSTOM << 3
        };

        template< class, class, class > friend class Segment;
        void _addSegment( S* segment );
        bool _removeSegment( S* segment );

        EQFABRIC_EXPORT virtual uint32_t commitNB(); //!< @internal
        bool _mapViewObjects();

        typedef net::CommandFunc< Canvas< CFG, C, S, L > > CmdFunc;
        bool _cmdNewSegment( net::Command& command );
        bool _cmdNewSegmentReply( net::Command& command );
    };

    template< class CFG, class C, class S, class L >
    std::ostream& operator << ( std::ostream&, const Canvas< CFG, C, S, L >& );
}
}
#endif // EQFABRIC_CANVAS_H

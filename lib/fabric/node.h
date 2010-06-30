
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 *               2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_NODE_H
#define EQFABRIC_NODE_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
    class Node;
    namespace server { class Node; }

namespace fabric
{
    struct NodePath;

    template< class C, class N, class P, class V > class Node : public Object
    {
    public:
        typedef std::vector< P* > Pipes;

        /** @name Data Access */
        //@{
        /** @return the config of this node. */
        C*       getConfig()       { return _config; }
        const C* getConfig() const { return _config; }

        /** @return vector of pipes */
        const Pipes& getPipes() const { return _pipes; }

        /**
         * @return true if all render tasks for this node are executed by the
         *         application process, false otherwise.
         * @internal
         */
        bool isApplicationNode() const { return _isAppNode; }
        void setApplicationNode( const bool isAppNode ); //!< @internal

        /** @return the index path to this node. @internal */
        EQFABRIC_EXPORT NodePath getPath() const;
        P* findPipe( const uint32_t id ); //!< @internal

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor );

        /** Const-version of accept(). */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor ) const;
        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in node.ipp
        /** Integer attributes. */
        enum IAttribute
        {
            /** <a href="http://www.equalizergraphics.com/documents/design/threads.html#sync">Threading model</a> */
            IATTR_THREAD_MODEL,
            IATTR_LAUNCH_TIMEOUT,         //!< Launch timeout
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        /** String attributes. */
        enum SAttribute
        {
            SATTR_LAUNCH_COMMAND,
            SATTR_FILL1,
            SATTR_FILL2,
            SATTR_ALL
        };

        /** Character attributes. */
        enum CAttribute
        {
            CATTR_LAUNCH_COMMAND_QUOTE,
            CATTR_FILL1,
            CATTR_FILL2,
            CATTR_ALL
        };

        EQFABRIC_EXPORT void setIAttribute( const IAttribute attr,
                                            const int32_t value );
        EQFABRIC_EXPORT int32_t getIAttribute( const IAttribute attr ) const;

        void setSAttribute( const SAttribute attr, const std::string& value );
        const std::string& getSAttribute( const SAttribute attr ) const;

        void setCAttribute( const CAttribute attr, const char value );
        char getCAttribute( const CAttribute attr ) const;

        static const std::string& getIAttributeString( const IAttribute attr );
        static const std::string& getSAttributeString( const SAttribute attr );
        static const std::string& getCAttributeString( const CAttribute attr );
        //@}

        EQFABRIC_EXPORT virtual void backup(); //!< @internal
        EQFABRIC_EXPORT virtual void restore(); //!< @internal
        void create( P** pipe ); //!< @internal
        void release( P* pipe ); //!< @internal

    protected:
        Node( C* parent );
        EQFABRIC_EXPORT virtual ~Node();

        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );
        
        EQFABRIC_EXPORT virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_EXPORT virtual void setDirty( const uint64_t bits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        enum DirtyBits
        {
            DIRTY_ATTRIBUTES      = Object::DIRTY_CUSTOM << 0,
            DIRTY_PIPES           = Object::DIRTY_CUSTOM << 1,
            DIRTY_MEMBER          = Object::DIRTY_CUSTOM << 2,
        };

        /** Pipe children. */
        Pipes _pipes;

        /** The parent config. */
        C* const _config;

        struct BackupData
        {
            /** Integer attributes. */
            int32_t iAttributes[IATTR_ALL];

            /** String attributes. */
            std::string sAttributes[SATTR_ALL];

            /** Character attributes. */
            char cAttributes[CATTR_ALL];
        }
            _data, _backup;

        bool _isAppNode; //!< execute render tasks in application process

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class, class, class > friend class Pipe;
        void _addPipe( P* pipe );
        bool _removePipe( P* pipe );

        EQFABRIC_EXPORT virtual uint32_t commitNB(); //!< @internal
        bool _mapNodeObjects() { return _config->mapNodeObjects(); }
    };
}
}

#endif // EQFABRIC_NODE_H



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

#ifndef EQFABRIC_ENTITY_H
#define EQFABRIC_ENTITY_H

#include <eq/fabric/Object.h>        // base class

namespace eq
{
namespace fabric
{
    class Entity : public Object
    {
    public:

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

        /** @name Data Access */
        //@{
        /** 
         * Return the set of tasks this channel might execute in the worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @return the tasks.
         * @warning Experimental - may not be supported in the future
         */
        uint32_t getTasks() const { return _tasks; }
        //@}

    protected: 
        /** Construct a new entity. @internal */
        EQFABRIC_EXPORT Entity( );

        /** Construct a new deep copy of a channel. @internal */
        EQFABRIC_EXPORT Entity( const Entity& from );

        /** Destruct the entity. @internal */
        virtual EQFABRIC_EXPORT ~Entity();

        /** Set thetasks this entity might potentially execute. @internal */
        void EQFABRIC_EXPORT setTasks( const uint32_t tasks );

        /** 
         * The changed parts of the entity since the last pack().
         *
         * Subclasses should define their own bits, starting at DIRTY_CUSTOM.
         */
        enum DirtyBits
        {
            DIRTY_ERROR      = Object::DIRTY_CUSTOM << 0,
            DIRTY_TASKS      = Object::DIRTY_CUSTOM << 1,
            // Leave room for binary-compatible patches
            DIRTY_CUSTOM     = Object::DIRTY_CUSTOM << 4
        };
        
        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

    private:

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The reason for the last error. */
        std::string _error;

    };

}
}
#endif // EQFABRIC_CANVAS_H

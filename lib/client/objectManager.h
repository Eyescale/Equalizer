
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OBJECTMANAGER_H
#define EQ_OBJECTMANAGER_H

#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/client/windowSystem.h>   // OpenGL types

namespace eq
{
    template< typename T >
    class EQ_EXPORT ObjectManager : public eqBase::NonCopyable
    {
    public:
        ObjectManager() {}
        virtual ~ObjectManager();

        void deleteAll();

        GLuint getList( const T& key );
        GLuint newList( const T& key );
        GLuint obtainList( const T& key );
        void   releaseList( const T& key );
        void   releaseList( const GLuint id );
        void   deleteList( const T& key );
        void   deleteList( const GLuint id );

    private:
        struct Object
        {
            GLuint   id;
            T        key;
            uint32_t refCount;
        };

        typedef stde::hash_map< GLuint, Object > ObjectIDHash;
        typedef stde::hash_map< T, Object* >     ObjectKeyHash;
        ObjectIDHash  _listsID;
        ObjectKeyHash _listsKey;
    };
}

#endif // EQ_OBJECTMANAGER_H


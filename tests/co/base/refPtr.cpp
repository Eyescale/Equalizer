
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <test.h>
#include <co/base/clock.h>
#include <co/base/refPtr.h>
#include <co/base/referenced.h>
#include <co/base/thread.h>
#include <iostream>

#ifdef CO_USE_BOOST
#  include <boost/intrusive_ptr.hpp>
#  include <boost/shared_ptr.hpp>
#endif

#ifdef CO_USE_BOOST_SERIALIZATION
#  include <boost/serialization/access.hpp>
#  include <boost/archive/text_oarchive.hpp>
#  include <boost/archive/text_iarchive.hpp>
#endif

#define NTHREADS 24
#define NREFS    300000

class Foo : public co::base::Referenced
{
public:
    Foo() {}

private:
#ifdef CO_USE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
    template< class Archive >
    void serialize( Archive& ar, unsigned int version )
    {
    }
#endif
    virtual ~Foo() {}
};

typedef co::base::RefPtr<Foo> FooPtr;
FooPtr foo;

class TestThread : public co::base::Thread
{
public:
    virtual void run()
        {
            FooPtr myFoo;
            for( size_t i = 0; i<NREFS; ++i )
            {
                myFoo = foo;
                foo   = myFoo;
                myFoo = 0;
            }
        }
};

#ifdef CO_USE_BOOST
typedef boost::intrusive_ptr<Foo> BoostPtr;
BoostPtr bFoo;

class BThread : public co::base::Thread
{
public:
    virtual void run()
        {
            BoostPtr myBoost;
            for( size_t i = 0; i<NREFS; ++i )
            {
                myBoost = bFoo;
                bFoo    = myBoost;
                myBoost = 0;
            }
        }
};

class Bar : public co::base::Referenced
{
public:
    Bar() {}
    virtual ~Bar() {}
};

typedef boost::shared_ptr<Bar> BarPtr;
BarPtr bBar;

class BarThread : public co::base::Thread
{
public:
    virtual void run()
        {
            BarPtr myBar;
            for( size_t i = 0; i<NREFS; ++i )
            {
                myBar = bBar;
                bBar  = myBar;
                myBar.reset();
            }
        }
};
#endif

int main( int argc, char **argv )
{
    foo = new Foo;

    TestThread threads[NTHREADS];
    co::base::Clock clock;
    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].join( ));

    const float time = clock.getTimef();
    std::cout << time << " ms for " << 3*NREFS << " reference operations in " 
              << NTHREADS << " threads (" << time/(3*NREFS*NTHREADS)*1000000
              << "ns/op)" << std::endl;

    TEST( foo->getRefCount() == 1 );

#ifdef CO_USE_BOOST
    bFoo = new Foo;
    BThread bThreads[NTHREADS];
    clock.reset();
    for( size_t i=0; i<NTHREADS; ++i )
        TEST( bThreads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( bThreads[i].join( ));

    const float bTime = clock.getTimef();
    std::cout << bTime << " ms for " << 3*NREFS << " boost::intrusive_ptr ops "
              << "in " << NTHREADS << " threads ("
              << bTime/(3*NREFS*NTHREADS)*1000000 << "ns/op)" << std::endl;

    TEST( bFoo->getRefCount() == 1 );

    boost::intrusive_ptr< Foo > boostFoo( foo.get( ));
    TEST( foo->getRefCount() == 2 );
    
    boostFoo = 0;
    TEST( foo->getRefCount() == 1 );

    bBar = BarPtr( new Bar );
    BarThread barThreads[NTHREADS];
    clock.reset();
    for( size_t i=0; i<NTHREADS; ++i )
        TEST( barThreads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( barThreads[i].join( ));

    const float barTime = clock.getTimef();
    std::cout << barTime << " ms for " << 3*NREFS <<" boost::shared_ptr ops in "
              << NTHREADS << " threads (" << barTime/(3*NREFS*NTHREADS)*1000000
              << "ns/op)" << std::endl;
#endif

    foo = 0;

#ifdef CO_USE_BOOST_SERIALIZATION
    FooPtr inFoo1 = new Foo;
    TEST( inFoo1->getRefCount() == 1 );
    FooPtr inFoo2 = inFoo1;
    TEST( inFoo2->getRefCount() == 2 );
    FooPtr outFoo1;
    std::stringstream stream;
    boost::archive::text_oarchive oar( stream );
    oar & inFoo1;
    boost::archive::text_iarchive iar( stream );
    iar & outFoo1;
    TEST( outFoo1->getRefCount() == 1 );
    FooPtr outFoo2 = outFoo1;
    TEST( outFoo2->getRefCount() == 2 );
#endif

    return EXIT_SUCCESS;
}


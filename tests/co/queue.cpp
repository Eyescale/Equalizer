
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Carsten Rohn <carsten.rohn@rtt.ag> 
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

#include <co/queueSlave.h>
#include <co/queueMaster.h>
#include <co/init.h>
#include <co/node.h>
#include <co/connectionDescription.h>
#include <co/base/sleep.h>


int main( int argc, char **argv )
{
    TEST( co::init( argc, argv ));

    co::LocalNodePtr node = new co::LocalNode;
    node->initLocal(argc, argv);

    co::QueueMaster* qm = new co::QueueMaster;
    co::QueueSlave* qs = new co::QueueSlave;

    node->registerObject(qm);
    node->mapObject(qs, qm->getID(), co::VERSION_FIRST);

    co::QueueItemPacket p1, p2, p3, p4;
    qm->push(p1);
    qm->push(p2);
    qm->push(p3);
    qm->push(p4);

    co::Command* c1 = qs->pop();
    co::Command* c2 = qs->pop();
    co::Command* c3 = qs->pop();
    co::Command* c4 = qs->pop();
    co::Command* c5 = qs->pop();

    TEST( c1 != 0 );
    TEST( c2 != 0 );
    TEST( c3 != 0 );
    TEST( c4 != 0 );
    TEST( c5 == 0 );

    c1->release();
    c2->release();
    c3->release();
    c4->release();

    co::base::sleep(1000);

    node->unmapObject( qs );
    node->deregisterObject( qm );

    delete qs;
    delete qm;

    node->close();

    co::exit();
    return EXIT_SUCCESS;
}
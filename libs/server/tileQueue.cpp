
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2011, Daniel Nachbaur<danielnachbaur@googlemail.com>
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

#include "tileQueue.h"

#include "compound.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{

TileQueue::TileQueue()
        : _compound( 0 )
        , _size( 0, 0 )
        //, _masterFrameData( 0 )
{
//    for( unsigned i = 0; i < NUM_EYES; ++i )
//        _frameData[i] = 0;
}

TileQueue::TileQueue( const TileQueue& from )
        : co::Object()
        , _compound( 0 )
        , _name( from._name )
        , _size( from._size )
//        , _data( from._data )
//        , _masterFrameData( 0 )
{
//    for( unsigned i = 0; i < NUM_EYES; ++i )
//        _frameData[i] = 0;
}

TileQueue::~TileQueue()
{
    _compound = 0;
}

void TileQueue::getInstanceData( co::DataOStream& os )
{
    //os << _inherit;
}

void TileQueue::applyInstanceData( co::DataIStream& is )
{
    //EQUNREACHABLE;
    //is >> _inherit;
}

void TileQueue::flush()
{
    unsetData();

    //while( !_datas.empty( ))
    //{
    //    FrameData* data = _datas.front();
    //    _datas.pop_front();
    //    getLocalNode()->deregisterObject( data );
    //    delete data;
    //}
}

void TileQueue::unsetData()
{
    //for( unsigned i = 0; i < NUM_EYES; ++i )
    //{
    //    _frameData[i] = 0;
    //    _inputFrames[i].clear();
    //}
}

void TileQueue::commitData()
{
    //if( !_masterFrameData ) // not used
    //    return;

    //for( unsigned i = 0; i< NUM_EYES; ++i )
    //{
    //    if( _frameData[i] )
    //    {
    //        if( _frameData[i] != _masterFrameData )
    //            _frameData[i]->_data = _masterFrameData->_data;

    //        _frameData[i]->commit();
    //    }
    //}
}

uint32_t TileQueue::commitNB( const uint32_t incarnation )
{
    //for( unsigned i = 0; i < NUM_EYES; ++i )
    //    _inherit.frameData[i] = _frameData[i];

    return co::Object::commitNB( incarnation );
}

//void TileQueue::cycleData( const uint32_t frameNumber, const Compound* compound )
//{
//    _masterFrameData = 0;
//    for( unsigned i = 0; i < NUM_EYES; ++i )
//    {
//        _inputFrames[i].clear();
//
//        if( !compound->isInheritActive( (eq::Eye)(1<<i) ))// eye pass not used
//        {
//            _frameData[i] = 0;
//            continue;
//        }
//
//        // reuse unused frame data
//        FrameData*     data    = _datas.empty() ? 0 : _datas.back();
//        const uint32_t latency = getAutoObsolete();
//        const uint32_t dataAge = data ? data->getFrameNumber() : 0;
//
//        if( data && dataAge < frameNumber-latency && frameNumber > latency )
//            // not used anymore
//            _datas.pop_back();
//        else // still used - allocate new data
//        {
//            data = new FrameData;
//
//            getLocalNode()->registerObject( data );
//            data->setAutoObsolete( 1 ); // current + in use by render nodes
//        }
//
//        data->setFrameNumber( frameNumber );
//    
//        _datas.push_front( data );
//        _frameData[i] = data;
//        if( !_masterFrameData )
//            _masterFrameData = data;
//    }
//}

//void TileQueue::addInputFrame( Frame* frame, const Compound* compound )
//{
//    for( unsigned i = 0; i < NUM_EYES; ++i )
//    {
//        // eye pass not used && no output frame for eye pass
//        if( compound->isInheritActive( (eq::Eye)(1<<i) ) &&  
//            _frameData[i] )     
//        {
//            frame->_frameData[i] = _frameData[i];
//            _inputFrames[i].push_back( frame );
//        }
//        else
//        {
//            frame->_frameData[i] = 0;           
//        }
//    }
//}

//co::ObjectVersion TileQueue::getDataVersion( const Eye eye ) const
//{
//    return co::ObjectVersion( _frameData[ co::base::getIndexOfLastBit( eye ) ] );
//}

std::ostream& operator << ( std::ostream& os, const TileQueue* tileQueue )
{
    if( !tileQueue )
        return os;
    
    os << co::base::disableFlush << "tileQueue" << std::endl;
    os << "{" << std::endl << co::base::indent;
      
    const std::string& name = tileQueue->getName();
    os << "name     \"" << name << "\"" << std::endl;

    const eq::Vector2i& size = tileQueue->getSize();
    os << "size     \"" << size << "\"" << std::endl;

    os << co::base::exdent << "}" << std::endl << co::base::enableFlush;
    return os;
}

}
}

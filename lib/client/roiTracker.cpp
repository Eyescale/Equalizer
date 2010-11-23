/* Copyright (c) 2009       Maxim Makhinya
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

#include "roiTracker.h"

namespace eq
{

ROITracker::Area::Area( const PixelViewport& pvp_,
                              uint32_t       lastSkip_,
                              uint32_t       skip_ )
    :pvp(      pvp_      )
    ,lastSkip( lastSkip_ )
    ,skip(     skip_     )
{
}


ROITracker::ROITracker()
    : _needsUpdate( false )
{
    _ticket   = reinterpret_cast< uint8_t* >( this );
    _prvFrame = new stde::hash_map< uint32_t, Stage >;
    _curFrame = new stde::hash_map< uint32_t, Stage >;
}


ROITracker::~ROITracker()
{
    delete _prvFrame;
    delete _curFrame;
    _prvFrame = 0;
    _curFrame = 0;
}


bool ROITracker::_returnPositive( uint8_t*& ticket )
{
    ticket = ++_ticket;
    _needsUpdate = true;
    return true;
}

bool ROITracker::useROIFinder( const PixelViewport& pvp,
                               const uint32_t       stage,
                               const uint128_t&     frameID,
                                     uint8_t*&      ticket )
{
    EQASSERT( !_needsUpdate );
    ticket = 0;

    const uint32_t pvpArea = pvp.getArea();
    if( pvpArea < 100 )
        return false;

    if( _lastFrameID != frameID ) // new frame
    {
        stde::hash_map< uint32_t, Stage >* tmp = _prvFrame;
        _prvFrame = _curFrame;
        _curFrame = tmp;
        _curFrame->clear();
        _lastFrameID = frameID;
    }

    _lastStage = stage;

    Stage& curStage = (*_curFrame)[ stage ];

    // check if proper stage is avaliable
    if( _prvFrame->find( stage ) == _prvFrame->end( )) // new stage
    {
        curStage.areas.push_back( Area( pvp ));
        return _returnPositive( ticket );
    }
    //else existing stage, try to find matching area

    const Area*    match     = 0;
          uint32_t bestArea  = 0;
    const Stage&   prvStage  = (*_prvFrame)[ stage ];
    for( uint32_t i = 0; i < prvStage.areas.size(); i++ )
    {
        PixelViewport tmp = prvStage.areas[i].pvp;
        tmp.intersect( pvp );
        const uint32_t area = tmp.getArea();
        if( area > bestArea )
        {
            bestArea = area;
            match    = &prvStage.areas[i];
            if( area == pvpArea ) // full match
                break;
        }
    }

    if( bestArea < pvpArea*2/3 ) // no proper match found, new area
    {
        curStage.areas.push_back( Area( pvp ));
        return _returnPositive( ticket );
    }
    // else good match

    if( match->skip == 0 ) // don't skip frame
    {
        curStage.areas.push_back( Area( pvp, match->lastSkip ));
        return _returnPositive( ticket );
    }
    //else skip frame

    curStage.areas.push_back( Area( pvp, match->lastSkip, match->skip-1 ));
    return false;
}


void ROITracker::updateDelay( const PixelViewports& pvps,
                              const uint8_t* ticket )
{
    EQASSERT( _needsUpdate );
    EQASSERTINFO( ticket == _ticket, "Wrong ticket" );

    if( ticket != _ticket )
    {
        EQERROR << "Wrong ticket" << std::endl;
        return;
    }

    uint32_t totalAreaFound = 0;
    for( uint32_t i = 0; i < pvps.size(); i++ )
        totalAreaFound += pvps[ i ].getArea();

    Area& area = (*_curFrame)[ _lastStage ].areas.back();
    if( totalAreaFound < area.pvp.getArea()*4/5 )
    {
        // ROI cutted enough, reset failure statistics
        area.lastSkip = 0;
    }else
    {
        // disable ROI for next frames, if it was failing before, 
        // increase number of frames to skip
        area.lastSkip = EQ_MIN( area.lastSkip*2 + 1, 64 );
        area.skip     = area.lastSkip;
    }
    _needsUpdate = false;
}

}



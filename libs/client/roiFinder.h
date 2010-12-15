/* Copyright (c) 2009 Maxim Makhinya
 *               2010 Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_ROI_FINDER_H
#define EQ_ROI_FINDER_H

#include "roiEmptySpaceFinder.h"
#include "roiTracker.h"

#include "image.h"   // member
#include "window.h"  // Window::ObjectManager

#include <vector>
#include <string>


namespace eq
{
    /**
     * Processes current rendering target and selects areas for read back.
     * @internal
     */
    class ROIFinder
    {
    public:
        ROIFinder();
        virtual ~ROIFinder() {}

        /**
         * Processes current rendering target and selects areas for read back.
         *
         * @param buffers   current buffers (BUFFER_COLOR / BUFFER_DEPTH).
         * @param pvp       viewport to analyse.
         * @param zoom      current zoom
         * @param stage     compositing stage (to track separate statistics).
         * @param frameID   ID of current frame (to track separate statistics).
         * @param glObjects object manager.
         * 
         * @return Areas for readback
         */
        PixelViewports findRegions( const uint32_t         buffers,
                                    const PixelViewport&   pvp,
                                    const Zoom&            zoom,
                                    const uint32_t         stage,
                                    const uint128_t&       frameID,
                                    Window::ObjectManager* glObjects );

        /** @return the GL function table, valid during readback. */
        const GLEWContext* glewGetContext() const
            { return _glObjects->glewGetContext(); }

    protected:

    private:
        struct Area;

        const void* _getInfoKey( ) const;

        /** Called from getReadbackInfo. Calculates per-block statistic before 
            actuall read-back */
        void _readbackInfo();

        /** Dumpes image that contain _mask and found regions */
        void _dumpDebug( const uint32_t stage = 0 );

        /** Clears masks, filles per-block occupancy _mask from _perBlockInfo,
            that was previously read-back from GPU in _readbackInfo */
        void _init( );

        /** For debugging purposes */
        void _fillWithColor( const PixelViewport& pvp, uint8_t* dst,
                             const uint8_t val );

        /** Updates dimensions and resizes arrays */
        void _resize( const PixelViewport& pvp );

        /** Histogram based based AABB calculation of a region. */
        PixelViewport _getObjectPVP( const PixelViewport& pvp,
                                     const uint8_t* src );

        /** Result is returned via _finalAreas array */
        uint8_t _splitArea( Area& area );

        /** Finds empty area in sub area. Used during optimal split search */
        void _updateSubArea( const uint8_t type );

        /** Find areas in current mask*/
        void _findAreas( PixelViewports& resultPVPs );

        /** Only used in debug build, to invalidate unused areas */
        void _invalidateAreas( Area* areas, uint8_t num );

        /** Dimensions of regions around currntly found hole. Used
            to estimate optimal split */
        struct Dims
        {
            uint8_t x1, x2, x3;
            uint8_t y1, y2, y3;
            uint8_t w1, w2, w3, w4, w5, w6, w7, w8;
            uint8_t h1, h2, h3, h4, h5, h6, h7, h8;
        } _dim;

        /** Describes region that is used to search holes withing */
        struct Area
        {
            Area(){};
            Area( PixelViewport pvp_ ) : emptySize( 0 ), pvp ( pvp_ ) {}

            int32_t       emptySize; //!< number of empty blocks
            PixelViewport pvp;       //!< PVP of area
            PixelViewport hole;      //!< largest hole

            bool          valid; //!< Used in debug build only
        };

        Area  _tmpAreas[17];  //!< possible arreas
        Area* _finalAreas[4]; //!< links to picked areas from _tmpAreas

        std::vector< Area > _areasToCheck;//!< Areas used to search for holes

        PixelViewport       _pvp;        //<! current, alligned to grid, PVP
        PixelViewport       _pvpOriginal;//<! original PVP

        ROIEmptySpaceFinder _emptyFinder;//!< class to search for holes

        int32_t _w;     //!< alligned to grid width 
        int32_t _h;     //!< alligned to grid height 
        int32_t _wh;    //!< _w * _h
        int32_t _wb;    //!< _w + 1 (only 1 block currently is used as border)
        int32_t _hb;    //!< _h + 1 (only 1 block currently is used as border)
        int32_t _wbhb;  //!< _wb * _wh (total number of blocks in _mask)

        Vectorub _tmpMask; //!< used only to dump found areas in _dumpDebug
        Vectorub _mask;    //!< mask of occupied blocks (main data)

        std::vector<float> _perBlockInfo; //!< buffer for data from GPU

        uint8_t _histX[256]; //!< histogram to find BB along X axis
        uint8_t _histY[256]; //!< histogram to find BB along Y axis

        Image _tmpImg;   //!< used for dumping debug info

        ROITracker _roiTracker; //!< disables ROI when ROI is inefficient

        /** The GL object manager, valid during a readback operation. */
        Window::ObjectManager* _glObjects;
    };
}

#endif // EQ_ROI_FINDER_H


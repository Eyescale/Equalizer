/* Copyright (c) 2009, Maxim Makhinya
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_ROI_TRACKER_H
#define EQ_ROI_TRACKER_H

#include <eq/api.h>
#include <eq/types.h>
#include <co/types.h>
#include <eq/fabric/pixelViewport.h> // member

#include <co/base/stdExt.h>          // member

#include <vector>
#include <string>

namespace eq
{
    /**
     * Tracks requested areas for ROI. Used to disable ROI when ROI 
     * fails.
     */
    class ROITracker
    {
    public:
        ROITracker();
        virtual ~ROITracker();

        /**
         * Has to be called once before any ROI calculation. Tels wether
         * ROIFinder should be executed for a particular region or not.
         * If true is returned, then updateDelay must be called once before
         * any next useROIFinder call. If false is returned, then updateDelay
         * shouldn't be called.
         *
         * This function uses _prvFrame areas data to match with current
         * pvp. If good intersection with one of previous areas is found,
         * it will use information from that area form previous frame to decide
         * wether it should skip ROIFinder execution or not. If good 
         * intersection is not found area cosidered new, and function returns
         * true.
         *
         * @param  pvp      same viewport that will be specifyed for FOIFinder
         * @param  stage    current assembling stage
         * @param  frameID  should be different for different frames
         * @param  ticket   should be stored by caller and given to updateDelay
         *
         * @return true if ROIFinder should be called for given region.
         */
        bool useROIFinder( const PixelViewport&   pvp,
                           const uint32_t        stage,
                           const uint128_t&      frameID,
                           uint8_t*&       ticket );

        /** 
         * Has to be called once after every positive result from useROIFinder.
         * Shouldn't be called otherwise
         *
         * It checks how much space ROIFinder was able to discard and if it is
         * not enough it will disable usage of ROIFinder for several next
         * frames. If ROIFinder was able to cut-off enought it will reset
         * failure statistics for that region.
         *
         * @param  pvps    result from ROIFinder
         * @param  ticket  value from useROIFinder
         */
        void updateDelay( const PixelViewports& pvps, const uint8_t* ticket );

    protected:

    private:
        /** Area of readback */
        struct Area
        {
            Area( const PixelViewport& pvp_,
                        uint32_t       lastSkip_ = 0,
                        uint32_t       skip_     = 0 );

            PixelViewport pvp;
            uint32_t      lastSkip; //!< Previousely skiped number of frames
            uint32_t      skip;     //!< Number of frames to skip ROIFinder
        };
        /** Set of readback areas per compositiong stage */
        struct Stage
        {
            std::vector< Area > areas;
        };
        /** Areas for different compositiong stages for current frame.
            This data if filled during useROIFinder calls, it uses 
            useROIFinder parameters and _prvFrame data as refference */
        stde::hash_map< uint32_t, Stage >* _curFrame;

        /** Areas for different compositiong stages for previous frame. */
        stde::hash_map< uint32_t, Stage >* _prvFrame;

        uint8_t* _ticket;//!< returned on getDelay, should match on updateDelay
        bool     _needsUpdate;//!< true after getDelay, false after updateDelay
        uint128_t _lastFrameID;//!< used to determine new frames
        uint32_t _lastStage;  //!< used in updateDelay to find last added area

        bool _returnPositive( uint8_t*& ticket );
    };
}

#endif //EQ_ROI_TRACKER_H


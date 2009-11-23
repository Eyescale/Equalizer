
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include "typedefs.h"

#include <eq/eq.h>


namespace eqPly
{
    class FrameData;
    class InitData;

    static const uint32_t primeNumberTable[100] = {
                    739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
                    811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
                    877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
                    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
                    1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
                    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
                    1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
                    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
                    1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
                    1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451
                    };

    /**
     * The rendering entity, updating a part of a Window.
     */
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent );

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameAssemble( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );
        virtual void frameStart( const uint32_t frameID,
                                 const uint32_t frameNumber );
        virtual void frameFinish( const uint32_t frameID,
                                  const uint32_t frameNumber );
        virtual void frameViewStart( const uint32_t frameID );
        virtual void frameViewFinish( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum() const;

    private:
        void _drawModel( const Model* model );
        void _drawOverlay();
        void _drawHelp();
        void _updateNearFar( const mesh::BoundingSphere& boundingSphere );
        void _initFrustum( eq::FrustumCullerf& frustum, 
                           const mesh::BoundingSphere& boundingSphere );

        bool _configInitAccumBuffer();
        bool _hasFinishFrames() const;

        const int32_t _getSampleSize() const
            { return ( getWindow()->getDrawableConfig().accumBits >= 64 ) ? 16 : 0; }

        /** the subpixel for this step. */
        eq::Vector2i _getJitterStep() const;
        eq::Vector2f _getJitter() const;

        const FrameData& _getFrameData() const;
        const Model*     _getModel();

        const Model* _model;
        uint32_t     _modelID;

        eq::Accum* _accum;

        uint32_t _jitterStep;
        uint32_t _totalSteps;
        uint32_t _subpixelStep;
        bool _needsTransfer;

        eq::PixelViewport _currentPVP;
    };
}



#endif // EQ_PLY_CHANNEL_H



/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__ORDER_ESTIMATOR_H
#define MASS_VOL__ORDER_ESTIMATOR_H

#include "renderNode.h"
#include "boxN.h"

#include <msv/types/types.h>
#include <msv/types/nonCopyable.h>

#include <memory> // std::auto_ptr


namespace massVolVis
{

class CameraParameters;
class GPUCacheManager;
class VolumeTreeBase;


class OrderEstimator : private NonCopyable
{
public:
    OrderEstimator();

    /**
     * Computes lists of blocks to render and to schedule for uploading
     * to GPU.
     * \param [out] renderNodes list of nodes to render (they are alread on GPU)
     * \param [out] desiredIds  list of nodes to load
     * \param       gpuManager  used to querry whether node is on GPU or not
     * \param       tree        current volume tree
     * \param       bb          BB of desired data x,y,z belong to [-1..1]
     * \param       cp          camera parameters (occlusion culling, order estimation)
     * \param       nodesMax    nodes budget
     * \param       tfHist      histogram of a current Transfer Function
     * \param       msMax       time budget
     */
    void compute(       RenderNodeVec&    renderNodes,
                        NodeIdPosVec&     desiredIds,
                  const GPUCacheManager&  gpuManager,
                  const VolumeTreeBase&   tree,
                  const Box_f&            bb,
                  const CameraParameters& cp,
                  const uint32_t          nodesMax,
                  const uint32_t          tfHist,
                  const float             msMax,
                  const uint8_t           maxTreeDepth,
                  const bool              frontToBack,
                        bool              useRenderingError,
                  const uint16_t          renderingError
                );

    const vecBoxN_f& getDataSplit( const uint32_t          n,
                                   const VolumeTreeBase&   tree,
                                   const CameraParameters& cp );

    void updateRenderingStats( const RenderNodeVec& renderNodes, double totalTimeMs );
private:

    float _estimateRenderingTime( const uint32_t size );

    vecBoxN_f _bbs;    //!< DB data split layout
    Box_f     _dataBB; //!< total data BB
    const std::auto_ptr< CameraParameters > _cpPtr; //!< Old camera parameters

    double _timePerBlockUnitSize;
};

} //namespace massVolVis

#endif //MASS_VOL__ORDER_ESTIMATOR_H


/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFERFUNCTIONGRAPHCORE_H
#define IVS_TRANSFERFUNCTIONGRAPHCORE_H

#include <cassert>
#include <fstream>
#include <ostream>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "TransferFunction.h"
#include "System/Types.h"

namespace ivs
{

class TransferFunctionGraphCore
{
public:
  enum Channel
  {
    R,
    G,
    B,
    A,
    AM,
    D,
    S
  };

  typedef boost::shared_ptr<TransferFunctionGraphCore> GraphCoreP;

  TransferFunctionGraphCore();
  virtual ~TransferFunctionGraphCore();

  virtual GraphCoreP clone() const = 0;

  virtual bool isSimilar(GraphCoreP _transferFunctionGraphCore) const;

  friend std::ostream& operator<<(std::ostream& os, const TransferFunctionGraphCore &_transferFunctionGraphCore);
  friend std::fstream& operator<<(std::fstream& fs, const TransferFunctionGraphCore &_transferFunctionGraphCore);
  friend std::fstream& operator>>(std::fstream& fs, TransferFunctionGraphCore &_transferFunctionGraphCore);

protected:
  float clamp(float _x) const;

  virtual void print(std::ostream &os) const = 0;
  virtual void write(std::fstream &fs) const = 0;
  virtual void read(std::fstream &fs)        = 0;
};

/**
 * A graph where the transfer function can be modified by freehand
 **/
/*class FreehandGraphCore: public TransferFunctionGraphCore
{
public:
  FreehandGraphCore(QWidget *_parent = 0);
  virtual ~FreehandGraphCore();

  virtual void initialize();

protected:
  void calculateUpdate(int _x, int _y, unsigned int _size, int &_lower,
                       int &_upper, float &_value) const;
  void updateCurve(int _x, int _y);
};*/

/**
 * Abstract interface for a class of graphs that are defined by anchor points
 **/
class AnchorGraphCore: public TransferFunctionGraphCore
{
public:
  static const float EPSILON;

  typedef std::vector<Vec2f>  AnchorVector;

  virtual void initialize(TransferFunctionPair *_transferFunction);

  virtual const AnchorVector &getAnchorVector(Channel _channel) const;
  virtual AnchorVector &getAnchorVector(Channel _channel);
  virtual const Vec2f &getAnchorPoint(Channel _channel, unsigned int _i) const;
  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              Channel _channel, unsigned int _i,
                              const Vec2f &_position)                           = 0;
  virtual void setAnchorPoints(TransferFunctionPair *_transferFunction,
                               Channel _channel,
                               const AnchorVector &_positions);
  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              const AnchorVector::iterator &_iterator,
                              Channel _channel, const Vec2f &_position)         = 0;
  virtual AnchorVector getCurve(Channel _channel) const                         = 0;
  virtual void setCurve(TransferFunctionPair *_transferFunction,
                        Channel _channel,
                        const AnchorVector &_parameters)                        = 0;

protected:
  virtual void print(std::ostream &os) const;
  virtual void write(std::fstream &fs) const;
  virtual void read(std::fstream &fs);
  virtual void updateCurve(TransferFunctionPair *_transferFunction, Channel _channel);
  virtual void writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride) = 0;

  AnchorVector                anchorPoints_[7];
};

/**
 * A graph where the transfer function is defined by a modifiable ramp
 **/
class RampGraphCore: public AnchorGraphCore
{
public:
  static AnchorVector transformAnchors(const AnchorVector &_anchors);
  static AnchorVector transformParameters(const AnchorVector &_parameters);

  RampGraphCore();
  virtual ~RampGraphCore();

  virtual GraphCoreP clone() const;

  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              Channel _channel, unsigned int _i,
                              const Vec2f &_position);
  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              const AnchorVector::iterator &_iterator,
                              Channel _channel, const Vec2f &_position);
  virtual AnchorVector getCurve(Channel _channel) const;
  virtual void setCurve(TransferFunctionPair *_transferFunction,
                        Channel _channel,
                        const AnchorVector &_parameters);

  virtual bool isSimilar(GraphCoreP _transferFunctionGraphCore) const;

protected:
  virtual void print(std::ostream &os) const;
  virtual void writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride);
};

/**
 * A graph where the transfer function is defined by a modifiable Gaussian
 **/
class GaussGraphCore: public AnchorGraphCore
{
public:
  static AnchorVector transformAnchors(const AnchorVector &_anchors);
  static AnchorVector transformParameters(const AnchorVector &_parameters);

  GaussGraphCore();
  virtual ~GaussGraphCore();

  virtual GraphCoreP clone() const;

  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              Channel _channel, unsigned int _i,
                              const Vec2f &_position);
  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              const AnchorVector::iterator &_iterator,
                              Channel _channel, const Vec2f &_position);
  virtual AnchorVector getCurve(Channel _channel) const;
  virtual void setCurve(TransferFunctionPair *_transferFunction,
                        Channel _channel,
                        const AnchorVector &_parameters);

  virtual bool isSimilar(GraphCoreP _transferFunctionGraphCore) const;

protected:
  virtual void print(std::ostream &os) const;
  virtual void writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride);
};

/**
 * A graph where the transfer function is defined by two modifiable Gaussian
 **/
class DoubleGaussGraphCore: public GaussGraphCore
{
public:
  DoubleGaussGraphCore();
  virtual ~DoubleGaussGraphCore();

  virtual GraphCoreP clone() const;

  virtual AnchorVector getCurve(Channel _channel) const;
  virtual void setCurve(TransferFunctionPair *_transferFunction,
                        Channel _channel,
                        const AnchorVector &_parameters);

  virtual bool isSimilar(GraphCoreP _transferFunctionGraphCore) const;

protected:
  virtual void print(std::ostream &os) const;
  virtual void writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride);
};

/**
 * A graph where the transfer function is defined by a modifiable ramp with an
 * additional Gaussian
 **/
class RampGaussGraphCore: public GaussGraphCore
{
public:
  RampGaussGraphCore();
  virtual ~RampGaussGraphCore();

  virtual GraphCoreP clone() const;

  virtual void setAnchorPoint(TransferFunctionPair *_transferFunction,
                              Channel _channel, unsigned int _i,
                              const Vec2f &_position);
  virtual AnchorVector getCurve(Channel _channel) const;
  virtual void setCurve(TransferFunctionPair *_transferFunction,
                        Channel _channel,
                        const AnchorVector &_parameters);

  virtual bool isSimilar(GraphCoreP _transferFunctionGraphCore) const;

protected:
  virtual void print(std::ostream &os) const;
  virtual void writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride);
};

inline std::ostream &operator<<(std::ostream &os, const TransferFunctionGraphCore &_transferFunctionGraphCore)
{
  _transferFunctionGraphCore.print(os);
  return os;
}

inline std::fstream &operator<<(std::fstream &fs, const TransferFunctionGraphCore &_transferFunctionGraphCore)
{
  _transferFunctionGraphCore.write(fs);
  return fs;
}

inline std::fstream &operator>>(std::fstream &fs, TransferFunctionGraphCore &_transferFunctionGraphCore)
{
  _transferFunctionGraphCore.read(fs);
  return fs;
}

inline bool TransferFunctionGraphCore::isSimilar(GraphCoreP) const
{
  return false;
}

inline float TransferFunctionGraphCore::clamp(float _x) const
{
  return _x < 0.0f ? 0.0f : (_x > 1.0f ? 1.0f : _x);
}

/*inline void FreehandGraphCore::initialize()
{}*/

inline const AnchorGraphCore::AnchorVector &AnchorGraphCore::getAnchorVector(Channel _channel) const
{
  return anchorPoints_[_channel];
}

inline AnchorGraphCore::AnchorVector &AnchorGraphCore::getAnchorVector(Channel _channel)
{
  return anchorPoints_[_channel];
}

inline const Vec2f &AnchorGraphCore::getAnchorPoint(Channel _channel, unsigned int _i) const
{
  assert(_i < anchorPoints_[_channel].size());
  return anchorPoints_[_channel][_i];
}

inline RampGraphCore::GraphCoreP RampGraphCore::clone() const
{
  return GraphCoreP(new RampGraphCore(*this));
}

inline void RampGraphCore::setAnchorPoint(TransferFunctionPair *_transferFunction,
    const AnchorVector::iterator &_iterator, Channel _channel,
    const Vec2f &_position)
{
  setAnchorPoint(_transferFunction, _channel, _iterator - anchorPoints_[_channel].begin(), _position);
}

inline RampGraphCore::AnchorVector RampGraphCore::getCurve(Channel _channel) const
{
  AnchorVector anchors = anchorPoints_[_channel];
  anchors.begin()->y() = 1.0f - anchors.begin()->y();
  (anchors.end() - 1)->y() = 1.0f - (anchors.end() - 1)->y();
  return transformAnchors(anchors);
}

inline void RampGraphCore::setCurve(TransferFunctionPair *_transferFunction,
                                    Channel _channel,
                                    const AnchorVector &_parameters)
{
  setAnchorPoints(_transferFunction, _channel, transformParameters(_parameters));
}

inline GaussGraphCore::GraphCoreP GaussGraphCore::clone() const
{
  return GraphCoreP(new GaussGraphCore(*this));
}

inline void GaussGraphCore::setAnchorPoint(TransferFunctionPair *_transferFunction,
    const AnchorVector::iterator &_iterator, Channel _channel,
    const Vec2f &_position)
{
  setAnchorPoint(_transferFunction, _channel, _iterator - anchorPoints_[_channel].begin(), _position);
}

inline GaussGraphCore::AnchorVector GaussGraphCore::getCurve(Channel _channel) const
{
  AnchorVector anchors = anchorPoints_[_channel];
  anchors.begin()->y() = 1.0f - anchors.begin()->y();
  (anchors.end() - 1)->y() = 1.0f - (anchors.end() - 1)->y();
  return GaussGraphCore::transformAnchors(anchors);
}

inline void GaussGraphCore::setCurve(TransferFunctionPair *_transferFunction,
                                     Channel _channel,
                                     const AnchorVector &_parameters)
{
  setAnchorPoints(_transferFunction, _channel, transformParameters(_parameters));
}

inline DoubleGaussGraphCore::GraphCoreP DoubleGaussGraphCore::clone() const
{
  return GraphCoreP(new DoubleGaussGraphCore(*this));
}

inline DoubleGaussGraphCore::AnchorVector DoubleGaussGraphCore::getCurve(Channel _channel) const
{
  AnchorVector gauss1, gauss2;
  gauss1.push_back(*anchorPoints_[_channel].begin());
  gauss1.push_back(*(anchorPoints_[_channel].begin() + 1));
  gauss2.push_back(*(anchorPoints_[_channel].begin() + 2));
  gauss2.push_back(*(anchorPoints_[_channel].begin() + 3));

  gauss1 = GaussGraphCore::transformAnchors(gauss1);
  gauss2 = GaussGraphCore::transformAnchors(gauss2);

  AnchorVector result = gauss1;
  result.push_back(*gauss2.begin());
  result.push_back(*(gauss2.end() - 1));

  return result;
}

inline void DoubleGaussGraphCore::setCurve(TransferFunctionPair *_transferFunction,
    Channel _channel,
    const AnchorVector &_parameters)
{
  AnchorVector gauss1, gauss2;
  gauss1.push_back(*_parameters.begin());
  gauss1.push_back(*(_parameters.begin() + 1));
  gauss2.push_back(*(_parameters.begin() + 2));
  gauss2.push_back(*(_parameters.begin() + 3));

  gauss1 = transformParameters(gauss1);
  gauss2 = transformParameters(gauss2);

  AnchorVector result = gauss1;
  result.push_back(*gauss2.begin());
  result.push_back(*(gauss2.end() - 1));

  setAnchorPoints(_transferFunction, _channel, result);
}

inline RampGaussGraphCore::GraphCoreP RampGaussGraphCore::clone() const
{
  return GraphCoreP(new RampGaussGraphCore(*this));
}

inline RampGaussGraphCore::AnchorVector RampGaussGraphCore::getCurve(Channel _channel) const
{
  AnchorVector ramp, gauss;
  ramp.push_back(*anchorPoints_[_channel].begin());
  ramp.push_back(*(anchorPoints_[_channel].begin() + 1));
  gauss.push_back(*(anchorPoints_[_channel].begin() + 2));
  gauss.push_back(*(anchorPoints_[_channel].begin() + 3));

  ramp = RampGraphCore::transformAnchors(ramp);
  gauss = GaussGraphCore::transformAnchors(gauss);

  AnchorVector result = ramp;
  result.push_back(*gauss.begin());
  result.push_back(*(gauss.end() - 1));

  return result;
}

inline void RampGaussGraphCore::setCurve(TransferFunctionPair *_transferFunction,
    Channel _channel,
    const AnchorVector &_parameters)
{
  AnchorVector ramp, gauss;
  ramp.push_back(*_parameters.begin());
  ramp.push_back(*(_parameters.begin() + 1));
  gauss.push_back(*(_parameters.begin() + 2));
  gauss.push_back(*(_parameters.begin() + 3));

  ramp = RampGraphCore::transformParameters(ramp);
  gauss = transformParameters(gauss);

  AnchorVector result = ramp;
  result.push_back(*gauss.begin());
  result.push_back(*(gauss.end() - 1));

  setAnchorPoints(_transferFunction, _channel, result);
}

}

#endif

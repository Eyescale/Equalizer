/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#include "TransferFunctionGraphCore.h"

#include <cmath>
#include <limits>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "System/Messages.h"
#include "System/Logger.h"

using namespace std;

namespace ivs
{

TransferFunctionGraphCore::TransferFunctionGraphCore()
{}

TransferFunctionGraphCore::~TransferFunctionGraphCore()
{}

/*FreehandGraphCore::FreehandGraphCore(QWidget *_parent):
  TransferFunctionGraph(_parent)
{}

FreehandGraphCore::~FreehandGraphCore()
{}

void FreehandGraphCore::calculateUpdate(int _x, int _y, unsigned int _size,
                                    int &_lower, int &_upper, float &_value) const
{
  float xFactor = static_cast<float>(_size - 1) / (width() - 1);
  float yFactor = 1.0f / (height() - 1);
  // calculate the affected range
  _lower = static_cast<int>(xFactor * _x);
  _upper = static_cast<int>(xFactor * (_x + 1));
  if(_upper > static_cast<int>(_size) - 1)
    _upper = _size - 1;
  _value = yFactor * (height() - 1 - _y);
}

void FreehandGraphCore::updateCurve(int _x, int _y)
{
  bool update = false;
  int lower, upper;
  float value;
  if(rgbaSize_ != 0 && rgba_ != 0)
  {
    calculateUpdate(_x, _y, rgbaSize_, lower, upper, value);

    // update the rgba curves
    for(int i = 0; i < 4; ++i)
      if(enabled_[i])
      {
        for(int j = lower; j <= upper; ++j)
          rgba_[4 * j + i] = value;
        update = true;
      }
  }

  if(sdaSize_ != 0 && sda_ != 0)
  {
    calculateUpdate(_x, _y, sdaSize_, lower, upper, value);

    // update the sda curves
    for(int i = 0; i < 3; ++i)
      if(enabled_[4 + i])
      {
        for(int j = lower; j <= upper; ++j)
          sda_[3 * j + i] = value;
        update = true;
      }
  }

  if(update)
    curveUpdate();
}*/

const float AnchorGraphCore::EPSILON = 0.05f;

void AnchorGraphCore::initialize(TransferFunctionPair *_transferFunction)
{
  for (int i = 0; i < 7; ++i)
    updateCurve(_transferFunction, static_cast<Channel>(i));
}

void AnchorGraphCore::setAnchorPoints(TransferFunctionPair *_transferFunction, Channel _channel,
                                      const std::vector<Vec2f> &_positions)
{
  assert(_positions.size() == anchorPoints_[_channel].size());
  for (unsigned int i = 0; i < _positions.size(); ++i)
  {
    Vec2f p(_positions[i].x(), 1.0f - _positions[i].y());
    // allow points outside of the area
    //p.clamp(0.0f, 1.0f);
    anchorPoints_[_channel][i] = p;
  }
  updateCurve(_transferFunction, _channel);
}

void AnchorGraphCore::print(ostream &_os) const
{
  for (int i = 0; i < 7; ++i)
  {
    _os << "Channel " << i << " ";
    BOOST_FOREACH(const Vec2f & v, anchorPoints_[i])
    _os << v;
  }
}

void AnchorGraphCore::write(fstream &_fs) const
{
  for (int i = 0; i < 7; ++i)
  {
    BOOST_FOREACH(const Vec2f & v, anchorPoints_[i])
    _fs << v;
    _fs << endl;
  }
}

void AnchorGraphCore::read(fstream &_fs)
{
  string s;
  for (int i = 0; i < 7; ++i)
  {
    getline(_fs, s);
    if (_fs.good())
    {
      anchorPoints_[i].clear();
      istringstream iss(s);
      char c;
      float x, y;
      iss >> c;
      while (iss.good())
      {
        if (c != '(')
          throw GET_ERROR_MESSAGE(ERR_INVALID_FILE_FORMAT) + "AnchorGraphCore";
        iss >> x >> c;
        if (c != ',')
          throw GET_ERROR_MESSAGE(ERR_INVALID_FILE_FORMAT) + "AnchorGraphCore";
        iss >> y >> c;
        if (c != ')')
          throw GET_ERROR_MESSAGE(ERR_INVALID_FILE_FORMAT) + "AnchorGraphCore";
        if (i != 4 && i != 5 && i != 6)
          anchorPoints_[i].push_back(Vec2f(x, y));
        //TODO: remove
        else
        {
          if (i == 4)
            anchorPoints_[i].push_back(Vec2f(0.5f, 0.6f));
          if (i == 5 || i == 6)
            anchorPoints_[i].push_back(Vec2f(0.5f, 0.5f));
        }
        iss >> c;
      }
      //TODO: remove
      if (i == 5 || i == 6)
      {
        anchorPoints_[i].begin()->x() = 0.0f;
        (anchorPoints_[i].end() - 1)->x() = 1.0f;
      }
    }
    else
      throw GET_ERROR_MESSAGE(ERR_INVALID_FILE_FORMAT) + "AnchorGraphCore";
  }
}

void AnchorGraphCore::updateCurve(TransferFunctionPair *_transferFunction, Channel _channel)
{
  if (_channel < 4)
    writeCurve(anchorPoints_[_channel], _transferFunction->first.getData() + _channel, _transferFunction->first.size(), 4);
  else
    writeCurve(anchorPoints_[_channel], _transferFunction->second.getData() + _channel - 4, _transferFunction->second.size(), 3);
}

RampGraphCore::AnchorVector RampGraphCore::transformAnchors(const AnchorVector &_anchors)
{
  AnchorVector parameters(_anchors.size());

  *parameters.begin() = Vec2f((_anchors.end() - 1)->x() - _anchors.begin()->x(),
                              0.5f * ((_anchors.end() - 1)->x() + _anchors.begin()->x()));
  *(parameters.end() - 1) = Vec2f((_anchors.end() - 1)->y() - _anchors.begin()->y(),
                                  _anchors.begin()->y());

  return parameters;
}

RampGraphCore::AnchorVector RampGraphCore::transformParameters(const AnchorVector &_parameters)
{
  AnchorVector anchors(_parameters.size());

  float a = (_parameters.end() - 1)->x();
  float b = _parameters.begin()->y();
  float c = _parameters.begin()->x();
  float d = (_parameters.end() - 1)->y();

  *anchors.begin() = Vec2f(-0.5f * c + b, d);
  *(anchors.end() - 1) = Vec2f(0.5f * c + b , d + a);

  return anchors;
}

RampGraphCore::RampGraphCore()
{
  for (int i = 0; i < 7; ++i)
  {
    anchorPoints_[i].clear();
    //anchorPoints_[i].push_back(Vec2f(0.0f, 1.0f));
    anchorPoints_[i].push_back(Vec2f(1.0f / 3.0f, 1.0f));
    anchorPoints_[i].push_back(Vec2f(2.0f / 3.0f, 0.0f));
    //anchorPoints_[i].push_back(Vec2f(1.0f, 0.0f));
  }
}

RampGraphCore::~RampGraphCore()
{}

void RampGraphCore::setAnchorPoint(TransferFunctionPair *_transferFunction,
                                   Channel _channel, unsigned int _i,
                                   const Vec2f &_position)
{
  Vec2f p = _position;
  p.y() = 1.0f - p.y();

  if (_i == 0)
  {
    if (p.x() > anchorPoints_[_channel][1].x())
      p.x() = anchorPoints_[_channel][1].x();
  }
  else if (_i == 1)
  {
    if (p.x() < anchorPoints_[_channel][0].x())
      p.x() = anchorPoints_[_channel][0].x();
  }
  anchorPoints_[_channel][_i] = p;
  updateCurve(_transferFunction, _channel);
}

bool RampGraphCore::isSimilar(GraphCoreP _transferFunctionGraphCore) const
{
  boost::shared_ptr<RampGraphCore> ggc = boost::dynamic_pointer_cast<RampGraphCore>(_transferFunctionGraphCore);
  if (ggc.get() != 0 &&
      getAnchorPoint(AnchorGraphCore::A, 0).distance(ggc->getAnchorPoint(AnchorGraphCore::A, 0)) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 1).distance(ggc->getAnchorPoint(AnchorGraphCore::A, 1)) < EPSILON)
    return true;
  return false;
}

void RampGraphCore::print(std::ostream &os) const
{
  AnchorVector curve = getCurve(AnchorGraphCore::A);
  os << "<tr><td>m:</td><td align=right>" << curve.begin()->x()
     << "</td></tr><tr><td>x:</td><td align=right>" << curve.begin()->y()
     << "</td></tr><tr><td>Amplitude:</td><td align=right>" << (curve.end() - 1)->x()
     << "</td></tr><tr><td>Offset:</td><td align=right>" << (curve.end() - 1)->y() << "</td></tr>";
}

void RampGraphCore::writeCurve(const AnchorVector &_anchors, float *_data,
                               unsigned int _count, unsigned int _stride)
{
  Vec2f start(_anchors.begin()->x() * (_count - 1), 1.0f - _anchors.begin()->y());
  Vec2f end((_anchors.end() - 1)->x() * (_count - 1), 1.0f - (_anchors.end() - 1)->y());
  float gradient = (end.y() - start.y()) / ((end.x() - start.x()));

  for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
  {
    if (i <= start.x())
      _data[j] = clamp(start.y());
    else if (i >= end.x())
      _data[j] = clamp(end.y());
    else
      _data[j] = clamp(start.y() +  gradient * (i - start.x()));
  }
}

GaussGraphCore::AnchorVector GaussGraphCore::transformAnchors(const AnchorVector &_anchors)
{
  AnchorVector parameters(_anchors.size());

  *(parameters.end() - 1) = Vec2f((_anchors.end() - 1)->y() - _anchors.begin()->y(),
                                  _anchors.begin()->y());

  // 1 / 256 because this is the smallest distinct distance
  float b = (_anchors.end() - 1)->x();
  float c = fabs((b - _anchors.begin()->x()) /
                 sqrt(-log(1.0f / (256.0f * (parameters.end() - 1)->x()))));

  *parameters.begin() = Vec2f(c, b);

  return parameters;
}

GaussGraphCore::AnchorVector GaussGraphCore::transformParameters(const AnchorVector &_parameters)
{
  AnchorVector anchors(_parameters.size());

  float a = (_parameters.end() - 1)->x();
  float b = _parameters.begin()->y();
  float c = _parameters.begin()->x();
  float d = (_parameters.end() - 1)->y();

  // 1 / 256 because this is the smallest distinct distance
  float discriminant = a != 0.0f ? c * sqrt(-log(1.0f / (256.0f * fabs(a)))) : b;

  *anchors.begin() = Vec2f(b - discriminant > 1.0f - b - discriminant ? b - discriminant : b + discriminant, d);
  *(anchors.end() - 1) = Vec2f(b, a + d);

  return anchors;
}

GaussGraphCore::GaussGraphCore()
{
  for (int i = 0; i < 7; ++i)
    anchorPoints_[i].clear();
  for (int i = 0; i < 3; ++i)
  {
    anchorPoints_[i].push_back(Vec2f(1.0f / 3.0f, 0.5f));
    anchorPoints_[i].push_back(Vec2f(0.5f, 0.5f));
  }
  anchorPoints_[3].push_back(Vec2f(1.0f / 3.0f, 1.0f));
  anchorPoints_[3].push_back(Vec2f(0.5f, 0.0f));
  anchorPoints_[4].push_back(Vec2f(1.0f / 3.0f, 2.0f / 3.0f));
  anchorPoints_[4].push_back(Vec2f(0.5f, 2.0f / 3.0f));
  anchorPoints_[5].push_back(Vec2f(1.0f / 3.0f, 0.5f));
  anchorPoints_[5].push_back(Vec2f(0.5f, 0.5f));
  anchorPoints_[6].push_back(Vec2f(1.0f / 3.0f, 0.5f));
  anchorPoints_[6].push_back(Vec2f(0.5f, 0.5f));
}

GaussGraphCore::~GaussGraphCore()
{
}

void GaussGraphCore::setAnchorPoint(TransferFunctionPair *_transferFunction,
                                    Channel _channel, unsigned int _i, const Vec2f &_position)
{
  assert(_i < anchorPoints_[_channel].size());
  Vec2f p(_position.x(), 1.0f - _position.y());
  // allow points outside of the area
  //p.clamp(0.0f, 1.0f);
  anchorPoints_[_channel][_i] = p;
  updateCurve(_transferFunction, _channel);
}

bool GaussGraphCore::isSimilar(GraphCoreP _transferFunctionGraphCore) const
{
  boost::shared_ptr<GaussGraphCore> ggc = boost::dynamic_pointer_cast<GaussGraphCore>(_transferFunctionGraphCore);
  if (ggc.get() != 0 &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 0).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 0).x()) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 0).y() == ggc->getAnchorPoint(AnchorGraphCore::A, 0).y() &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 1).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 1).x()) < EPSILON)
    return true;
  return false;
}

void GaussGraphCore::print(std::ostream &os) const
{
  AnchorVector curve = getCurve(AnchorGraphCore::A);
  os << "<tr><td>&sigma;:</td><td align=right>" << curve.begin()->x()
     << "</td></tr><tr><td>&mu;:</td><td align=right>" << curve.begin()->y()
     << "</td></tr><tr><td>Amplitude:</td><td align=right>" << (curve.end() - 1)->x()
     << "</td></tr><tr><td>Offset:</td><td align=right>" << (curve.end() - 1)->y() << "</td></tr>";
}

void GaussGraphCore::writeCurve(const AnchorVector &_anchors, float *_data,
                                unsigned int _count, unsigned int _stride)
{
  const Vec2f &start = *_anchors.begin();
  const Vec2f &end = *(_anchors.end() - 1);

  float radius = end.x() - start.x();
  float height = start.y() - end.y();
  Vec2f offset(end.x(), 1.0f - start.y());
  if (radius != 0.0f)
  {
    float radiusFactor = 6.907755279f / (radius * radius);

    for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
    {
      float x = static_cast<float>(i) / (_count - 1) - offset.x();
      _data[j] = clamp(height * exp(- x * x * radiusFactor) + offset.y());
    }
  }
  else
  {
    for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
      _data[j] = clamp(i == start.x() * (_count - 1) ? height + offset.y() : offset.y());
  }
}

DoubleGaussGraphCore::DoubleGaussGraphCore()
{
  for (int i = 0; i < 7; ++i)
    anchorPoints_[i].clear();
  for (int i = 0; i < 3; ++i)
  {
    anchorPoints_[i].push_back(Vec2f(0.0f, 0.5f));
    anchorPoints_[i].push_back(Vec2f(0.25f, 0.5f));
    anchorPoints_[i].push_back(Vec2f(0.5f, 0.5f));
    anchorPoints_[i].push_back(Vec2f(0.75f, 0.5f));
  }
  anchorPoints_[3].push_back(Vec2f(0.0f, 1.0f));
  anchorPoints_[3].push_back(Vec2f(0.25f, 0.0f));
  anchorPoints_[3].push_back(Vec2f(0.5f, 1.0f));
  anchorPoints_[3].push_back(Vec2f(0.75f, 0.0f));

  anchorPoints_[4].push_back(Vec2f(0.0f, 2.0f / 3.0f));
  anchorPoints_[4].push_back(Vec2f(0.25f, 2.0f / 3.0f));
  anchorPoints_[4].push_back(Vec2f(0.5f, 2.0f / 3.0f));
  anchorPoints_[4].push_back(Vec2f(0.75f, 2.0f / 3.0f));

  anchorPoints_[5].push_back(Vec2f(0.0f, 0.5f));
  anchorPoints_[5].push_back(Vec2f(0.25f, 0.5f));
  anchorPoints_[5].push_back(Vec2f(0.5f, 0.5f));
  anchorPoints_[5].push_back(Vec2f(0.75f, 0.5f));

  anchorPoints_[6].push_back(Vec2f(0.0f, 0.5f));
  anchorPoints_[6].push_back(Vec2f(0.25f, 0.5f));
  anchorPoints_[6].push_back(Vec2f(0.5f, 0.5f));
  anchorPoints_[6].push_back(Vec2f(0.75f, 0.5f));
}

DoubleGaussGraphCore::~DoubleGaussGraphCore()
{
}

bool DoubleGaussGraphCore::isSimilar(GraphCoreP _transferFunctionGraphCore) const
{
  boost::shared_ptr<GaussGraphCore> ggc = boost::dynamic_pointer_cast<GaussGraphCore>(_transferFunctionGraphCore);
  if (ggc.get() != 0 &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 0).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 0).x()) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 0).y() == ggc->getAnchorPoint(AnchorGraphCore::A, 0).y() &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 1).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 1).x()) < EPSILON &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 2).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 2).x()) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 2).y() == ggc->getAnchorPoint(AnchorGraphCore::A, 2).y() &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 3).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 3).x()) < EPSILON)
    return true;
  return false;
}

void DoubleGaussGraphCore::print(std::ostream &os) const
{
  AnchorVector curve = getCurve(AnchorGraphCore::A);
  os << "<tr><td>&sigma;:</td><td align=right>" << curve.begin()->x()
     << "</td></tr><tr><td>&mu;:</td><td align=right>" << curve.begin()->y()
     << "</td></tr><tr><td>Amplitude:</td><td align=right>" << (curve.begin() + 1)->x()
     << "</td></tr><tr><td>Offset:</td><td align=right>" << (curve.begin() + 1)->y();
  os << "</td></tr><tr><td>&sigma; 2:</td><td align=right>" << (curve.begin() + 2)->x()
     << "</td></tr><tr><td>&mu; 2:</td><td align=right>" << (curve.begin() + 2)->y()
     << "</td></tr><tr><td>Amplitude 2:</td><td align=right>" << (curve.begin() + 3)->x()
     << "</td></tr><tr><td>Offset 2:</td><td align=right>" << (curve.begin() + 3)->y() << "</td></tr>";
}

void DoubleGaussGraphCore::writeCurve(const AnchorVector &_anchors, float *_data,
                                      unsigned int _count, unsigned int _stride)
{
  const Vec2f &start1 = *_anchors.begin();
  const Vec2f &end1 = *(_anchors.begin() + 1);
  const Vec2f &start2 = *(_anchors.end() - 2);
  const Vec2f &end2 = *(_anchors.end() - 1);

  float radius1 = end1.x() - start1.x(),
        radius2 = end2.x() - start2.x();
  float height1 = start1.y() - end1.y(),
        height2 = start2.y() - end2.y();
  Vec2f offset1(end1.x(), 1.0f - start1.y()),
        offset2(end2.x(), 1.0f - start2.y());

  float radiusFactor1 = radius1 != 0.0f ? 6.907755279f / (radius1 * radius1) : 0.0f,
        radiusFactor2 = radius2 != 0.0f ? 6.907755279f / (radius2 * radius2) : 0.0f;

  for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
  {
    float x1 = static_cast<float>(i) / (_count - 1) - offset1.x(),
          x2 = static_cast<float>(i) / (_count - 1) - offset2.x();
    _data[j] = clamp(std::max(radiusFactor1 != 0.0f ?
                              height1 * exp(- x1 * x1 * radiusFactor1) + offset1.y() :
                              (i == start1.x() * (_count - 1) ? height1 + offset1.y() : offset1.y()),
                              radiusFactor2 != 0.0f ?
                              height2 * exp(- x2 * x2 * radiusFactor2) + offset2.y() :
                              (i == start2.x() * (_count - 1) ? height2 + offset2.y() : offset2.y())
                             ));
  }
}

RampGaussGraphCore::RampGaussGraphCore()
{
  for (int i = 0; i < 7; ++i)
  {
    anchorPoints_[i].clear();
    // ramp
    anchorPoints_[i].push_back(Vec2f(1.0f / 3.0f, 1.0f));
    anchorPoints_[i].push_back(Vec2f(2.0f / 3.0f, 0.0f));

    // Gaussian
    anchorPoints_[i].push_back(Vec2f(0.4f, 1.0f));
    anchorPoints_[i].push_back(Vec2f(0.5f, 0.0f));
  }
}

RampGaussGraphCore::~RampGaussGraphCore()
{}

void RampGaussGraphCore::setAnchorPoint(TransferFunctionPair *_transferFunction,
                                        Channel _channel, unsigned int _i, const Vec2f &_position)
{
  assert(_i < anchorPoints_[_channel].size());

  if (_i < 2)
  {
    Vec2f p = _position;
    p.y() = 1.0f - p.y();

    if (_i == 0)
    {
      if (p.x() > anchorPoints_[_channel][1].x())
        p.x() = anchorPoints_[_channel][1].x();
    }
    else if (_i == 1)
    {
      if (p.x() < anchorPoints_[_channel][0].x())
        p.x() = anchorPoints_[_channel][0].x();
    }
    anchorPoints_[_channel][_i] = p;
  }
  else
  {
    Vec2f p(_position.x(), 1.0f - _position.y());
    p.clamp(0.0f, 1.0f);
    anchorPoints_[_channel][_i] = p;
  }
  updateCurve(_transferFunction, _channel);
}

bool RampGaussGraphCore::isSimilar(GraphCoreP _transferFunctionGraphCore) const
{
  boost::shared_ptr<RampGraphCore> ggc = boost::dynamic_pointer_cast<RampGraphCore>(_transferFunctionGraphCore);
  if (ggc.get() != 0 &&
      getAnchorPoint(AnchorGraphCore::A, 1).distance(ggc->getAnchorPoint(AnchorGraphCore::A, 1)) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 2).distance(ggc->getAnchorPoint(AnchorGraphCore::A, 2)) < EPSILON &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 4).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 4).x()) < EPSILON &&
      getAnchorPoint(AnchorGraphCore::A, 4).y() == ggc->getAnchorPoint(AnchorGraphCore::A, 5).y() &&
      fabs(getAnchorPoint(AnchorGraphCore::A, 5).x() - ggc->getAnchorPoint(AnchorGraphCore::A, 5).x()) < EPSILON)
    return true;
  return false;
}

void RampGaussGraphCore::print(std::ostream &os) const
{
  AnchorVector curve = getCurve(AnchorGraphCore::A);
  os << "<tr><td>m:</td><td align=right>" << curve.begin()->x()
     << "</td></tr><tr><td>x:</td><td align=right>" << curve.begin()->y()
     << "</td></tr><tr><td>Amplitude:</td><td align=right>" << (curve.begin() + 1)->x()
     << "</td></tr><tr><td>Offset:</td><td align=right>" << (curve.begin() + 1)->y();
  os << "</td></tr><tr><td>&sigma;:</td><td align=right>" << (curve.begin() + 2)->x()
     << "</td></tr><tr><td>&mu;:</td><td align=right>" << (curve.begin() + 2)->y()
     << "</td></tr><tr><td>Amplitude 2:</td><td align=right>" << (curve.begin() + 3)->x()
     << "</td></tr><tr><td>Offset 2:</td><td align=right>" << (curve.begin() + 3)->y() << "</td></tr>";
}

void RampGaussGraphCore::writeCurve(const AnchorVector &_anchors, float *_data, unsigned int _count, unsigned int _stride)
{
  const Vec2f &gStart = *(_anchors.end() - 2);
  const Vec2f &gEnd = *(_anchors.end() - 1);

  Vec2f rStart = *_anchors.begin();
  Vec2f rEnd = *(_anchors.begin() + 1);
  rStart.y() = 1.0f - rStart.y();
  rEnd.y() = 1.0f - rEnd.y();

  float radius = gEnd.x() - gStart.x();
  float height = gStart.y() - gEnd.y();
  Vec2f offset(gEnd.x(), 1.0f - gStart.y());

  float radiusFactor = radius != 0.0f ? 6.907755279f / (radius * radius) : 0.0f;
  float gradient = (rEnd.y() - rStart.y()) / ((rEnd.x() - rStart.x()) * (_count - 1));

  for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
  {
    float x = static_cast<float>(i) / (_count - 1);
    float xo = x - offset.x();

    _data[j] = clamp(std::max(radiusFactor != 0.0f ?
                              height * exp(- xo * xo * radiusFactor) + offset.y() :
                              (i == gStart.x() * (_count - 1) ? height + offset.y() : offset.y()),
                              x <= rStart.x() ? rStart.y() :
                              (x >= rEnd.x() ? rEnd.y() :
                               rStart.y() + gradient * (i - rStart.x() * (_count - 1)))));
  }
}

}

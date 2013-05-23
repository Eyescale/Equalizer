/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 *         David Steiner    (steiner@ifi.uzh.ch)
 **/

#include "TransferFunctionGraph.h"

#include <cmath>
#include <limits>

#include <boost/bind.hpp>

#include <QPolygon>

using namespace std;

namespace ivs
{

TransferFunctionGraph::TransferFunctionGraph(QWidget *arg):
  QWidget(arg),
  lastPointOk_(false),
  histogram_(0),
  histogramSize_(0),
  transferFunction_(0)
{
  colors_[0] = Qt::red;
  colors_[1] = Qt::green;
  colors_[2] = Qt::blue;
  colors_[3] = Qt::black;
  colors_[4] = Qt::darkCyan;
  colors_[5] = Qt::darkMagenta;
  colors_[6] = Qt::darkYellow;

  QPalette pal;
  pal.setColor(QPalette::Background, Qt::white);
  setPalette(pal);
  setBackgroundRole(QPalette::Window);
  setAutoFillBackground(true);
}

TransferFunctionGraph::~TransferFunctionGraph()
{}

void TransferFunctionGraph::paintHistogram(QPainter &_painter)
{
  if (histogramSize_ == 0)
    return;
  QPolygon line;
  line.resize(histogramSize_ + 2);
  unsigned int max = 0;
  for (unsigned int i = 1; i < histogramSize_; ++i)
    if (histogram_[i] > max)
      max = histogram_[i];
  float xFactor = static_cast<float>(width()) / histogramSize_;
  float yFactor = (static_cast<float>(height() - 1)) / max;
  line.setPoint(0, 0, height() - 1);
  for (unsigned int i = 0; i < histogramSize_; ++i)
    line.setPoint(i + 1, static_cast<int>(xFactor * i), height() - 1 - (static_cast<int>(yFactor * histogram_[i])));
  line.setPoint(histogramSize_ + 1, width() - 1, height() - 1);
  _painter.drawPolygon(line);
}

void TransferFunctionGraph::paintCurve(QPainter &_painter, const float *_data,
                                       unsigned int _count, unsigned int _stride)
{
  QPolygon line;
  line.resize(_count);
  float xFactor = static_cast<float>(width() - 1) / (_count - 1);
  float yFactor = height() - 1;
  for (unsigned int i = 0, j = 0; i < _count; ++i, j += _stride)
    line.setPoint(i, static_cast<int>(xFactor * i), height() - 1 - (static_cast<int>(yFactor * _data[j])));
  _painter.drawPolyline(line);
}

FreehandGraph::FreehandGraph(QWidget *_parent):
  TransferFunctionGraph(_parent)
{}

FreehandGraph::~FreehandGraph()
{}

void FreehandGraph::mouseMoveEvent(QMouseEvent *arg)
{
  if (arg->buttons() & Qt::LeftButton &&
      ((transferFunction_->first.size() != 0 && transferFunction_->first.getData() != 0) ||
       (transferFunction_->second.size() != 0 && transferFunction_->second.getData() != 0)))
  {
    QPoint clippedPoint = getClippedPoint(arg->pos());
    int posX = clippedPoint.x();
    int posY = clippedPoint.y();
    if (lastPointOk_ && abs(posX - lastPoint_.x()) > 1)
    {
      // Bresenham for interpolating between the last two captured points
      int lastX = lastPoint_.x(), lastY = lastPoint_.y();
      bool steep = abs(posY - lastY) > abs(posX - lastX);
      if (steep)
      {
        std::swap(lastX, lastY);
        std::swap(posX, posY);
      }
      if (lastX > posX)
      {
        std::swap(lastX, posX);
        std::swap(lastY, posY);
      }
      int deltaX = posX - lastX, deltaY = abs(posY - lastY);
      int error = deltaX / 2;
      int yStep = lastY < posY ? 1 : -1;
      for (int i = lastX, j = lastY; i <= posX; ++i)
      {
        if (steep)
          updateCurve(j, i);
        else
          updateCurve(i, j);
        error -= deltaY;
        if (error < 0)
        {
          j += yStep;
          error += deltaX;
        }
      }
    }
    else
      updateCurve(posX,  posY);

    lastPoint_   = clippedPoint;
    lastPointOk_ = true;
    update();
  }
}

void FreehandGraph::mousePressEvent(QMouseEvent *_event)
{
  if (_event->button() & Qt::LeftButton)
  {
    lastPoint_ = getClippedPoint(_event->pos());
    updateCurve(lastPoint_.x(), lastPoint_.y());
    lastPointOk_ = true;
    update();
  }
}

void FreehandGraph::calculateUpdate(int _x, int _y, unsigned int _size,
                                    int &_lower, int &_upper, float &_value) const
{
  float xFactor = static_cast<float>(_size - 1) / (width() - 1);
  float yFactor = 1.0f / (height() - 1);
  // calculate the affected range
  _lower = static_cast<int>(xFactor * _x);
  _upper = static_cast<int>(xFactor * (_x + 1));
  if (_upper > static_cast<int>(_size) - 1)
    _upper = _size - 1;
  _value = yFactor * (height() - 1 - _y);
}

void FreehandGraph::updateCurve(int _x, int _y)
{
  bool b = false;
  int low, up;
  float value;
  if (transferFunction_->first.size() != 0 && transferFunction_->first.getData() != 0)
  {
    calculateUpdate(_x, _y, transferFunction_->first.size(), low, up, value);

    // update the rgba curves
    for (int i = 0; i < 4; ++i)
      if (enabled_[i])
      {
        for (int j = low; j <= up; ++j)
          transferFunction_->first.getData()[4 * j + i] = value;
        b = true;
      }
  }

  if (transferFunction_->second.size() != 0 && transferFunction_->second.getData() != 0)
  {
    calculateUpdate(_x, _y, transferFunction_->second.size(), low, up, value);

    // update the sda curves
    for (int i = 0; i < 3; ++i)
      if (enabled_[4 + i])
      {
        for (int j = low; j <= up; ++j)
          transferFunction_->second.getData()[3 * j + i] = value;
        b = true;
      }
  }

  if (b)
    curveUpdate();
}

void FreehandGraph::paint(QPainter &_painter)
{
  QBrush   brush(Qt::lightGray);
  _painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
  _painter.setPen(Qt::lightGray);
  _painter.setBrush(brush);
  paintHistogram(_painter);
  if (transferFunction_->first.size() != 0 && transferFunction_->first.getData() != 0)
    for (int i = 0; i < 4; ++i)
///      if (enabled_[i])
      {
        _painter.setPen(colors_[i]);
        paintCurve(_painter, transferFunction_->first.getData() + i, transferFunction_->first.size(), 4);
      }

  if (transferFunction_->second.size() != 0 && transferFunction_->second.getData() != 0)
    for (int i = 0; i < 3; ++i)
///      if (enabled_[i + 4])
      {
        _painter.setPen(colors_[i + 4]);
        paintCurve(_painter, transferFunction_->second.getData() + i, transferFunction_->second.size(), 3);
      }
}

const int   AnchorGraph::INNER_RADIUS   = 3;
const int   AnchorGraph::OUTER_RADIUS   = 6;

AnchorGraph::AnchorGraph(AnchorGraphCoreP _anchorGraphCore, QWidget *_parent):
  TransferFunctionGraph(_parent),
  graphCore_(_anchorGraphCore)
{}

AnchorGraph::~AnchorGraph()
{}

void AnchorGraph::initialize()
{
  graphCore_->initialize(transferFunction_);
  curveUpdate(true);
  update();
}

void AnchorGraph::mouseMoveEvent(QMouseEvent *_event)
{
  if (_event->buttons() & Qt::LeftButton &&
      ((transferFunction_->first.size() != 0 && transferFunction_->first.getData() != 0) ||
       (transferFunction_->second.size() != 0 && transferFunction_->second.getData() != 0)))
  {
    int posX = _event->pos().x();
    int posZ = _event->pos().y();

    Vec2f delta(static_cast<float>(posX - lastPoint_.x()) / (width() - 1),
                static_cast<float>(posZ - lastPoint_.y()) / (height() - 1));

    if (lastPointOk_)
    {
      AnchorIndexVector::iterator i;
      for (i = currentAnchors_.begin(); i != currentAnchors_.end(); ++i)
      {
        Vec2f p = *(i->first) + delta;
        p.y() = 1.0f - p.y();
        graphCore_->setAnchorPoint(transferFunction_, i->first, i->second, p);
      }
    }
    // allow points outside of the definition scope for curves in different parameter space
    lastPoint_   = _event->pos(); //getClippedPoint(_event->pos());
    lastPointOk_ = true;
    if (!currentAnchors_.empty())
      curveUpdate();
    update();
  }
}

void AnchorGraph::mousePressEvent(QMouseEvent *_event)
{
  if (_event->button() & Qt::LeftButton)
  {
    // allow points outside of the definition scope for curves in different parameter space
    lastPoint_ = _event->pos(); //getClippedPoint(_event->pos());
    for (int i = 0; i < 7; ++i)
      if (enabled_[i])
      {
        AnchorVector::iterator j;
        for (j = graphCore_->getAnchorVector(static_cast<Channel>(i)).begin();
             j != graphCore_->getAnchorVector(static_cast<Channel>(i)).end(); ++j)
        {
          QPoint p = getTransformedPoint(*j);
          if (lastPoint_.x() <= p.x() + OUTER_RADIUS &&
              lastPoint_.x() >= p.x() - OUTER_RADIUS &&
              lastPoint_.y() <= p.y() + OUTER_RADIUS &&
              lastPoint_.y() >= p.y() - OUTER_RADIUS)
            currentAnchors_.push_back(AnchorIndex(j, static_cast<Channel>(i)));
        }
      }
    lastPointOk_ = true;
    update();
  }
}

void AnchorGraph::paintAnchors(QPainter &_painter, int _i)
{
  QBrush brush(colors_[_i]);
  AnchorVector::const_iterator j;
  for (j = graphCore_->getAnchorVector(static_cast<Channel>(_i)).begin();
       j != graphCore_->getAnchorVector(static_cast<Channel>(_i)).end(); ++j)
  {
    _painter.setBrush(brush);
    _painter.drawEllipse(getTransformedPoint(*j), INNER_RADIUS, INNER_RADIUS);
    _painter.setBrush(Qt::NoBrush);
    _painter.drawEllipse(getTransformedPoint(*j), OUTER_RADIUS, OUTER_RADIUS);
  }
}

void AnchorGraph::paint(QPainter &_painter)
{
  QBrush   brush(Qt::lightGray);
  _painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
  _painter.setPen(Qt::lightGray);
  _painter.setBrush(brush);
  paintHistogram(_painter);
  if (transferFunction_->first.size() != 0 && transferFunction_->first.getData() != 0)
    for (int i = 0; i < 4; ++i)
///      if (enabled_[i])
      {
        _painter.setPen(colors_[i]);
        paintCurve(_painter, transferFunction_->first.getData() + i, transferFunction_->first.size(), 4);
        paintAnchors(_painter, i);
      }

  if (transferFunction_->second.size() != 0 && transferFunction_->second.getData() != 0)
    for (int i = 0; i < 3; ++i)
///      if (enabled_[i + 4])
      {
        _painter.setPen(colors_[i + 4]);
        paintCurve(_painter, transferFunction_->second.getData() + i, transferFunction_->second.size(), 3);
        paintAnchors(_painter, i + 4);
      }
}

}

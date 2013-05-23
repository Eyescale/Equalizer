/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFERFUNCTIONGRAPH_H
#define IVS_TRANSFERFUNCTIONGRAPH_H

#include <utility>
#include <vector>

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <QPainter>
#include <QMouseEvent>
#include <QWidget>

#include "TransferFunctionGraphCore.h"
#include "System/Types.h"

namespace ivs
{
///class IvsWidget;

/**
 * The transfer function graph widget. A value array for the color mapping and
 * a value array for the material mapping can be provided. The transfer function
 * graph widget shows these mappings and allows editing them. Editing directly
 * affects the arrays provided. No local copy is stored.
 **/
class TransferFunctionGraph: public QWidget
{
  Q_OBJECT

public:
  typedef TransferFunctionGraphCore::Channel           Channel;
  typedef boost::shared_ptr<TransferFunctionGraphCore> GraphCoreP;

  TransferFunctionGraph(QWidget *_parent = 0);
  virtual ~TransferFunctionGraph();

  TransferFunctionPair *getTransferFunction();
  void setTransferFunction(TransferFunctionPair *_transferFunction);

  void setHistogram(const unsigned int *_histogram, unsigned int _size);

  virtual const GraphCoreP getGraphCore() const    = 0;
  virtual GraphCoreP getGraphCore()                = 0;
  virtual void setGraphCore(GraphCoreP _graphCore) = 0;

  virtual void initialize() = 0;

Q_SIGNALS:
  void curveUpdate(bool _immediate = false);

public Q_SLOTS:
  void setStateR(bool _checked);
  void setStateG(bool _checked);
  void setStateB(bool _checked);
  void setStateA(bool _checked);
  void setStateS(bool _checked);
  void setStateD(bool _checked);
  void setStateAm(bool _checked);

protected:
  QPoint getClippedPoint(const QPoint &_point) const;
  void paintHistogram(QPainter &_painter);
  void paintCurve(QPainter &_painter, const float *_data, unsigned int _count,
                  unsigned int _stride = 0);

  QPoint                      lastPoint_;
  bool                        lastPointOk_;
  bool                        enabled_[7];
  const unsigned int         *histogram_;
  unsigned int                histogramSize_;
  QColor                      colors_[7];
  TransferFunctionPair       *transferFunction_;
};

/**
 * A graph where the transfer function can be modified by freehand
 **/
class FreehandGraph: public TransferFunctionGraph
{
  Q_OBJECT

public:
  FreehandGraph(QWidget *_parent = 0);
  virtual ~FreehandGraph();

  virtual const GraphCoreP getGraphCore() const;
  virtual GraphCoreP getGraphCore();
  virtual void setGraphCore(GraphCoreP _graphCore);

  virtual void initialize();

protected:
  virtual void mouseMoveEvent(QMouseEvent *_event);
  virtual void mousePressEvent(QMouseEvent *_event);
  virtual void mouseReleaseEvent(QMouseEvent *_event);
  virtual void paintEvent(QPaintEvent *_event);

  void calculateUpdate(int _x, int _y, unsigned int _size, int &_lower,
                       int &_upper, float &_value) const;
  void updateCurve(int _x, int _y);
  void paint(QPainter &_painter);
};

/**
 * A graph where the transfer function is defined by a modifiable ramp
 **/
class AnchorGraph: public TransferFunctionGraph
{
  Q_OBJECT

public:
  typedef boost::shared_ptr<AnchorGraphCore>           AnchorGraphCoreP;
  typedef AnchorGraphCore::AnchorVector                AnchorVector;

  AnchorGraph(AnchorGraphCoreP _anchorGraphCore, QWidget *_parent = 0);
  virtual ~AnchorGraph();

  virtual void initialize();

  virtual const GraphCoreP getGraphCore() const;
  virtual GraphCoreP getGraphCore();
  virtual void setGraphCore(GraphCoreP _graphCore);

  virtual const AnchorVector &getAnchorVector(Channel _channel) const;
  virtual AnchorVector &getAnchorVector(Channel _channel);
  virtual const Vec2f &getAnchorPoint(Channel _channel, unsigned int _i) const;
  virtual void setAnchorPoint(Channel _channel, unsigned int _i, const Vec2f &_position);
  virtual void setAnchorPoints(Channel _channel, const std::vector<Vec2f> &_positions);

  virtual AnchorVector getCurve(Channel _channel) const;
  virtual void setCurve(Channel _channel, const std::vector<Vec2f> &_parameters);

protected:
  typedef std::pair<AnchorGraphCore::AnchorVector::iterator, Channel> AnchorIndex;
  typedef std::vector<AnchorIndex>                                    AnchorIndexVector;

  static const int   INNER_RADIUS;
  static const int   OUTER_RADIUS;

  virtual void mouseMoveEvent(QMouseEvent *_event);
  virtual void mousePressEvent(QMouseEvent *_event);
  virtual void mouseReleaseEvent(QMouseEvent *_event);
  virtual void paintEvent(QPaintEvent *_event);

  QPoint getTransformedPoint(const Vec2f &_point, float _xOffset = 0.0f, float _yOffset = 0.0f) const;
  void paintAnchors(QPainter &_painter, int _i);
  void paint(QPainter &_painter);

  AnchorIndexVector                currentAnchors_;
  AnchorGraphCoreP                 graphCore_;
};

inline TransferFunctionPair *TransferFunctionGraph::getTransferFunction()
{
  return transferFunction_;
}

inline void TransferFunctionGraph::setTransferFunction(TransferFunctionPair *_transferFunction)
{
  transferFunction_ = _transferFunction;
}

inline void TransferFunctionGraph::setHistogram(const unsigned int *_histogram, unsigned int _size)
{
  histogram_ = _histogram;
  histogramSize_ = _size;
}

inline void TransferFunctionGraph::setStateR(bool _checked)
{
  enabled_[0] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateG(bool _checked)
{
  enabled_[1] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateB(bool _checked)
{
  enabled_[2] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateA(bool _checked)
{
  enabled_[3] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateS(bool _checked)
{
  enabled_[6] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateD(bool _checked)
{
  enabled_[5] = _checked;
  update();
}

inline void TransferFunctionGraph::setStateAm(bool _checked)
{
  enabled_[4] = _checked;
  update();
}

inline QPoint TransferFunctionGraph::getClippedPoint(const QPoint &_point) const
{
  return QPoint(std::max(0, std::min(width() - 1, _point.x())),
                std::max(0, std::min(height() - 1, _point.y())));
}

inline const FreehandGraph::GraphCoreP FreehandGraph::getGraphCore() const
{
  return GraphCoreP();
}

inline FreehandGraph::GraphCoreP FreehandGraph::getGraphCore()
{
  return GraphCoreP();
}

inline void FreehandGraph::setGraphCore(GraphCoreP)
{}

inline void FreehandGraph::initialize()
{}

inline void FreehandGraph::mouseReleaseEvent(QMouseEvent *_event)
{
  if (_event->button() & Qt::LeftButton)
    lastPointOk_ = false;
}

inline void FreehandGraph::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  paint(painter);
}

inline const AnchorGraph::GraphCoreP AnchorGraph::getGraphCore() const
{
  return graphCore_;
}

inline AnchorGraph::GraphCoreP AnchorGraph::getGraphCore()
{
  return graphCore_;
}

inline void AnchorGraph::setGraphCore(GraphCoreP _graphCore)
{
  lastPointOk_ = false;
  currentAnchors_.clear();
  graphCore_ = boost::dynamic_pointer_cast<AnchorGraphCore>(_graphCore);
}

inline const AnchorGraph::AnchorVector &AnchorGraph::getAnchorVector(Channel _channel) const
{
  return graphCore_->getAnchorVector(_channel);
}

inline AnchorGraph::AnchorVector &AnchorGraph::getAnchorVector(Channel _channel)
{
  return graphCore_->getAnchorVector(_channel);
}

inline const Vec2f &AnchorGraph::getAnchorPoint(Channel _channel, unsigned int _i) const
{
  return graphCore_->getAnchorPoint(_channel, _i);
}

inline void AnchorGraph::setAnchorPoint(Channel _channel, unsigned int _i, const Vec2f &_position)
{
  graphCore_->setAnchorPoint(transferFunction_, _channel, _i, _position);
  curveUpdate(true);
  update();
}

inline void AnchorGraph::setAnchorPoints(Channel _channel, const std::vector<Vec2f> &_positions)
{
  graphCore_->setAnchorPoints(transferFunction_, _channel, _positions);
  curveUpdate(true);
  update();
}

inline AnchorGraph::AnchorVector AnchorGraph::getCurve(Channel _channel) const
{
  return graphCore_->getCurve(_channel);
}

inline void AnchorGraph::setCurve(Channel _channel, const std::vector<Vec2f> &_parameters)
{
  graphCore_->setCurve(transferFunction_, _channel, _parameters);
  curveUpdate(true);
  update();
}

inline void AnchorGraph::mouseReleaseEvent(QMouseEvent *_event)
{
  if (_event->button() & Qt::LeftButton)
  {
    lastPointOk_ = false;
    currentAnchors_.clear();
  }
}

inline void AnchorGraph::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  paint(painter);
}

inline QPoint AnchorGraph::getTransformedPoint(const Vec2f &_point, float _xOffset, float _yOffset) const
{
  return QPoint(_point.x() * (width() - 1) + _xOffset + 0.5f,
                _point.y() * (height() - 1) + _yOffset + 0.5f);
}

}

#endif

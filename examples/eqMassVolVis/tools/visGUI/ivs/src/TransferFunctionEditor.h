/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 * edited: David Steiner (steiner@ifi.uzh.ch)
 **/

#ifndef IVS_TRANSFERFUNCTIONEDITOR_H
#define IVS_TRANSFERFUNCTIONEDITOR_H

#include <cstring>

#include <QDir>
#include <QWidget>

#include "System/Config.h"
#include "System/Timer.h"

#ifndef IVS_LINUX
#  include "GUI/ui_TransferFunctionEditor.h"
#else
#  include "ui_TransferFunctionEditor.h"
#endif

#include "TransferFunction.h"
#include "TransferFunctionGraphCore.h"
#include "TransferFunctionGraph.h"

#include <msv/types/types.h> // byte


namespace massVolGUI
{
    class MainWindow;
}

namespace ivs
{

/**
 *
 **/
class TransferFunctionEditor: public QDialog
{
  Q_OBJECT

public:
  enum Graph
  {
    FREEHAND,
    RAMP,
    GAUSS,
    DOUBLE_GAUSS,
    RAMP_GAUSS
  };

  typedef TransferFunctionGraphCore::Channel Channel;

  TransferFunctionEditor( massVolGUI::MainWindow *_mainWnd, QWidget *_parent = 0 );
  ~TransferFunctionEditor();

  virtual void setUpdatesEnabled(bool _enable) ;

  /** set a new transfer function. 0 is not allowed. **/
  virtual void setTransferFunction(TransferFunctionPair *_transferFunction);

  /** set a histogram which will be shown as background **/
  void setHistogram(const unsigned int *_histogram, unsigned int _size);

  /** get the type of the current graph used for editing **/
  Graph getGraphType() const;

  /** set the type of graph to be used for editing **/
  void setGraphType(Graph _graph);

  /** get the current graph used for editing **/
  TransferFunctionGraph *getGraph();

  /** enable or disable channels **/
  void setChannel(Channel _channel, bool _enabled);

  bool getRangeAutoUpate() const;
  void setRangeAutoUpdate(bool _rangeAutoUpdate);
  
///  IvsWidget *getIvsWidget();

  virtual void update() ;

public Q_SLOTS:
  virtual void run();
  virtual void accept();
  virtual void reject();
  virtual void updateCurve(bool _immediate = false);
  virtual void changePreview(bool _checked);
  virtual void changeRangeAutoUpdate(bool _checked);
  virtual void triggerMainWndUpdate();

protected:
  virtual void closeEvent(QCloseEvent *_event);

protected Q_SLOTS:
  virtual void changeRenderer();
  virtual void changeGraph(int _i);
  virtual void changeTransferFunctionType(int _i);
  virtual void loadTransferFunction(bool _checked);
  virtual void saveTransferFunction(bool _checked);

private:
    
  void initializeGraph(TransferFunctionGraph *_transferFunctionGraph);
  void backup(unsigned int _size,
              const float *_data,
              unsigned int &_backupSize,
              float *&_backupData,
              unsigned int _components);
  void restore(unsigned int _backupSize,
               const float *_backupData,
               unsigned int _size,
               float *_data,
               unsigned int _components);
  void backupTransferFunction();
  void restoreTransferFunction();

  QPoint                      position_;
  QDir                        directory_;
  TransferFunctionGraph      *transferFunctionGraph_;
  Ui::TransferFunctionEditor  ui_;
  TransferFunctionPair       *transferFunction_;
  float                      *rgba_, *sda_;
  const unsigned int         *histogram_;
  unsigned int                rgbaSize_, sdaSize_, histogramSize_;
  sys::Timer                  timer_;
  massVolGUI::MainWindow     *mainWnd_;

  bool                  updateMaxSet_;
};

inline void TransferFunctionEditor::setHistogram(const unsigned int *_histogram, unsigned int _size)
{
  histogram_     = _histogram;
  histogramSize_ = _size;
  transferFunctionGraph_->setHistogram(_histogram, _size);
}

inline void TransferFunctionEditor::setUpdatesEnabled(bool _enable)
{
  QWidget::setUpdatesEnabled(_enable);
}

inline TransferFunctionEditor::Graph TransferFunctionEditor::getGraphType() const
{
  return static_cast<Graph>(ui_.graphComboBox_->currentIndex());
}

inline void TransferFunctionEditor::setGraphType(Graph _graph)
{
  if (ui_.graphComboBox_->currentIndex() != _graph)
    ui_.graphComboBox_->setCurrentIndex(static_cast<int>(_graph));
}

inline TransferFunctionGraph *TransferFunctionEditor::getGraph()
{
  return transferFunctionGraph_;
}

inline bool TransferFunctionEditor::getRangeAutoUpate() const
{
  return ui_.rangeAutoUpdateCheckBox_->isChecked();
}

inline void TransferFunctionEditor::setRangeAutoUpdate(bool _rangeAutoUpdate)
{
  return ui_.rangeAutoUpdateCheckBox_->setChecked(_rangeAutoUpdate);
}

/*inline IvsWidget *TransferFunctionEditor::getIvsWidget()
{
  return ivsWidget_;
}*/


}

#endif

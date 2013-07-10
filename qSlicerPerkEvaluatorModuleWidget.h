
#ifndef __qSlicerPerkEvaluatorModuleWidget_h
#define __qSlicerPerkEvaluatorModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerPerkEvaluatorTransformBufferWidget.h"
#include "qSlicerPerkEvaluatorMessagesWidget.h"
#include "qSlicerPerkEvaluatorModuleExport.h"

class qSlicerPerkEvaluatorModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PERKEVALUATOR_EXPORT qSlicerPerkEvaluatorModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPerkEvaluatorModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorModuleWidget();

  unsigned int UpdateStatus;

public slots:


  void OnPlaybackSliderChanged( double value );
  void OnPlaybackNextClicked();
  void OnPlaybackPrevClicked();
  void OnPlaybackBeginClicked();
  void OnPlaybackEndClicked();
  void OnPlaybackPlayClicked();
  void OnPlaybackStopClicked();

  void OnTimeout();

  void OnMarkBeginClicked();
  void OnMarkEndClicked();

  void OnAnalyzeClicked();

  void OnBodyModelNodeSelected();
  void OnNeedleReferenceSelected();
  void OnNeedleOrientationChanged( QAbstractButton* newOrientationButton );
  
  void UpdateGUI();

protected:
  QScopedPointer<qSlicerPerkEvaluatorModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorModuleWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorModuleWidget);
 
  
  double TimerIntervalSec;
  QTimer* Timer;
  
};

#endif


#ifndef __qSlicerPerkEvaluatorModuleWidget_h
#define __qSlicerPerkEvaluatorModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerPerkEvaluatorTransformBufferWidget.h"
#include "qSlicerPerkEvaluatorMessagesWidget.h"
#include "qSlicerPerkEvaluatorTransformRolesWidget.h"
#include "qSlicerPerkEvaluatorAnatomyRolesWidget.h"
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

public slots:

  void OnPlaybackSliderChanged( double value );
  void OnPlaybackNextClicked();
  void OnPlaybackPrevClicked();
  void OnPlaybackBeginClicked();
  void OnPlaybackEndClicked();
  void OnPlaybackPlayClicked();
  void OnPlaybackStopClicked();

  void OnTimeout();

  void OnMarkBeginEdited();
  void OnMarkBeginClicked();
  void OnMarkEndEdited();
  void OnMarkEndClicked();

  void OnMetricsDirectoryClicked();

  void OnAnalyzeClicked();
  void OnClipboardClicked();
  void OnTraceTrajectoriesChanged();

  void OnBodyModelNodeSelected();
  void OnNeedleReferenceSelected();
  void OnNeedleOrientationChanged( QAbstractButton* newOrientationButton );
  
  void updateWidget();
  void resetWidget();
  void clearWidget();

protected:
  QScopedPointer<qSlicerPerkEvaluatorModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void setupEmbeddedWidgets();

  QTimer* PlaybackTimer;
  double PlaybackTimerIntervalSec;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorModuleWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorModuleWidget);
  
};

#endif

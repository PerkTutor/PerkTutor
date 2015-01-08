
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

  void OnAnalyzeClicked();
  void OnClipboardClicked();

  void OnMarkBeginChanged();
  void OnMarkBeginClicked();
  void OnMarkEndChanged();
  void OnMarkEndClicked();

  void OnMetricsDirectoryClicked();
  void OnAutoUpdateMeasurementRangeToggled();
  void OnAutoUpdateTransformRolesToggled();

  void onTissueModelChanged( vtkMRMLNode* node );
  void onNeedleTransformChanged( vtkMRMLNode* node );
  void onNeedleOrientationChanged( QAbstractButton* newOrientationButton );
  
  void updateWidget();
  void resetWidget();
  void clearWidget();

  void mrmlNodeChanged( vtkMRMLNode* );
  void updateWidgetFromMRMLNode();

protected:
  QScopedPointer<qSlicerPerkEvaluatorModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void setupEmbeddedWidgets();
  virtual void enter();

  QTimer* PlaybackTimer;
  double PlaybackTimerIntervalSec;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorModuleWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorModuleWidget);
  
};

#endif
